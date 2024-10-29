#include "httpserver.hpp"
#include "json.hpp"

HTTPServer::HTTPServer()
{
    this->weatherData = nullptr;
    this->weatherDataLock = nullptr;
    this->mainThreadPriority = 0;
}

HTTPServer::HTTPServer(std::map<Date, WeatherDataPoint>* weatherData, bool* weatherDataLock, s32 mainThreadPriority)
{
    this->weatherData = weatherData;
    this->weatherDataLock = weatherDataLock;
    this->mainThreadPriority = mainThreadPriority;
}

HTTPServer::~HTTPServer()
{
    this->EndServer();
}

void HTTPServer::StartServer()
{
    this->serverThread = threadCreate(RunServerHelper, (void*)this, 4096, this->mainThreadPriority-1, 1, false);
}

void HTTPServer::EndServer()
{
    this->runThread = false;
    threadJoin(this->serverThread, U64_MAX);
    threadFree(this->serverThread);
}

void HTTPServer::RunServer()
{
    int serverFd, newSocket;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);

    // Create socket file descriptor
    if ((serverFd = socket(AF_INET, SOCK_STREAM, IPPROTO_IP)) == 0)
    {
        std::cout << "Socket failed." << std::endl;
        return;
    }

    // Attach socket to port
    if (setsockopt(serverFd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)))
    {
        std::cout << "setsockopt failed." << std::endl;
        return;
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    // Bind socket to port
    if (bind(serverFd, (struct sockaddr *)&address, sizeof(address)) < 0)
    {
        std::cout << "Bind failed." << std::endl;
        return;
    }

    // Listen for connections
    if (listen(serverFd, 3) < 0)
    {
        std::cout << "Listen error." << std::endl;
        return;
    }

    while (this->runThread)
    {
        // Most of this boilerplate was copied from https://www.geeksforgeeks.org/socket-programming-in-cpp/ lol
        fd_set readfds;
        struct timeval timeout;

        // Clear set
        FD_ZERO(&readfds);

        FD_SET(serverFd, &readfds);

        // 0.1 seconds
        timeout.tv_sec = 0;
        timeout.tv_usec = 100000;

        int activity = select(serverFd + 1, &readfds, nullptr, nullptr, &timeout);

        if (activity < 0)
        {
            std::cout << "Select error." << std::endl;
            return;
        }
        else if (activity == 0)
            continue;
        
        // A client is trying to connect
        if (FD_ISSET(serverFd, &readfds))
        {
            if ((newSocket = accept(serverFd, (struct sockaddr*)&address, (socklen_t*)&addrlen)) < 0)
            {
                std::cout << "Accept error." << std::endl;
                return;
            }

            // Handle request
            const int bufferSize = 1024;
            char buffer[bufferSize];
            read(newSocket, buffer, bufferSize);

            std::string request(buffer);
            std::string method = request.substr(0, request.find(" "));

            std::string path = request.substr(request.find(" ") + 1);

            // Separate args from path
            std::string args = "";
            if (path.find("?") != std::string::npos)
            {
                args = path.substr(path.find("?"));
                args = args.substr(0, args.find(" "));
            }

            path = path.substr(0, path.find("?"));
            path = path.substr(0, path.find(" "));
            
            std::string data = request.substr(request.find("\r\n\r\n") + 4);

            // Wait until weather data not being used
            while (*this->weatherDataLock) {}
            *this->weatherDataLock = true;

            if (method == "GET")
            {
                try
                {
                    std::vector<WeatherDataPoint> mapAsVector = weatherDataMapToVector(*this->weatherData);

                    // Get data from date, index, date range, or index range
                    nlohmann::json json;
                    std::vector<WeatherDataPoint> includedData;

                    if (args.find("start_date") != std::string::npos && args.find("end_date") != std::string::npos) // Date range used
                    {
                        std::vector<std::string> argVector = splitString(args.substr(1), '&');

                        // Get range
                        Date startDate, endDate;
                        for (std::string s : argVector)
                        {
                            std::vector<std::string> argPair = splitString(s, '=');
                            if (argPair.at(0) == "start_date")
                            {
                                std::vector<std::string> dateParts = splitString(argPair.at(1), '-');
                                if (dateParts.size() == 1) // index
                                {
                                    startDate = mapAsVector.at(stoi(argPair.at(1))).date;
                                }
                                else if (dateParts.size() == 3) // date
                                {
                                    startDate = Date(stoi(dateParts[0]), stoi(dateParts[1]), stoi(dateParts[2]));
                                }
                                else
                                {
                                    throw std::exception();
                                }
                            }
                            else if (argPair.at(0) == "end_date")
                            {
                                std::vector<std::string> dateParts = splitString(argPair.at(1), '-');
                                if (dateParts.size() == 1) // index
                                {
                                    endDate = mapAsVector.at(stoi(argPair.at(1))).date;
                                }
                                else if (dateParts.size() == 3) // date
                                {
                                    endDate = Date(stoi(dateParts[0]), stoi(dateParts[1]), stoi(dateParts[2]));
                                }
                                else
                                {
                                    throw std::exception();
                                }
                            }
                        }

                        // Get data and convert to JSON
                        includedData = getDataFromRange(mapAsVector, startDate, endDate);
                        to_json(json, includedData);
                    }
                    else // Single date used
                    {
                        std::vector<std::string> dateParts = splitString(path.substr(1), '-');
                        Date date;
                        if (dateParts.size() == 1) // Index
                        {
                            date = mapAsVector.at(stoi(path.substr(1))).date;
                        }
                        else if (dateParts.size() == 3) // Date
                        {
                            date = Date(stoi(dateParts[0]), stoi(dateParts[1]), stoi(dateParts[2]));
                        }
                        else
                        {
                            throw std::exception();
                        }

                        // Get data and convert to JSON
                        to_json(json, (*this->weatherData).at(date));
                    }

                    // Prune JSON of unwanted values
                    if (args.find("data") != std::string::npos)
                    {
                        std::vector<std::string> argVector = splitString(args.substr(1), '&');

                        for (std::string s : argVector)
                        {
                            std::vector<std::string> argPair = splitString(s, '=');
                            if (argPair.at(0) != "data") continue;

                            if (json.find("date") != json.end()) // Data is at base level, so there is only one data point
                            {
                                if (argPair.at(1).find("weather_code") == std::string::npos)
                                    json.erase("weather_code");
                                if (argPair.at(1).find("temperature_max") == std::string::npos)
                                    json.erase("temperature_max");
                                if (argPair.at(1).find("temperature_min") == std::string::npos)
                                    json.erase("temperature_min");
                                if (argPair.at(1).find("precipitation_sum") == std::string::npos)
                                    json.erase("precipitation_sum");
                                if (argPair.at(1).find("wind_speed_max") == std::string::npos)
                                    json.erase("wind_speed_max");
                                if (argPair.at(1).find("precipitation_probability_max") == std::string::npos)
                                    json.erase("precipitation_probability_max");
                            }
                            else // Multi-point, check all sub-elements
                            {
                                for (unsigned int i = 0; i < json.size(); i++)
                                {
                                    if (argPair.at(1).find("weather_code") == std::string::npos)
                                        json.at(i).erase("weather_code");
                                    if (argPair.at(1).find("temperature_max") == std::string::npos)
                                        json.at(i).erase("temperature_max");
                                    if (argPair.at(1).find("temperature_min") == std::string::npos)
                                        json.at(i).erase("temperature_min");
                                    if (argPair.at(1).find("precipitation_sum") == std::string::npos)
                                        json.at(i).erase("precipitation_sum");
                                    if (argPair.at(1).find("wind_speed_max") == std::string::npos)
                                        json.at(i).erase("wind_speed_max");
                                    if (argPair.at(1).find("precipitation_probability_max") == std::string::npos)
                                        json.at(i).erase("precipitation_probability_max");
                                }
                            }

                            break;
                        }
                    }

                    // Send JSON
                    std::string jsonStr = json.dump();

                    std::string response = "HTTP/1.1 200 OK\r\nContent-Length: " + std::to_string(jsonStr.size()) + "\r\n\r\n" + jsonStr;
                    send(newSocket, response.c_str(), response.size(), 0);
                }
                catch(const std::exception& e)
                {
                    std::string response = "HTTP/1.1 400 Bad Request\r\nContent-Length: 0";
                    send(newSocket, response.c_str(), response.size(), 0);
                }
            }
            else if (method == "POST")
            {
                try
                {
                    // Check if data provided is valid
                    if (path.size() <= 1 || data.find("weather_code") == std::string::npos || data.find("temperature_max") == std::string::npos || 
                        data.find("temperature_min") == std::string::npos || data.find("precipitation_sum") == std::string::npos || 
                        data.find("wind_speed_max") == std::string::npos || data.find("precipitation_probability_max") == std::string::npos)
                        throw std::exception();
                    
                    // Create data point and parse data
                    WeatherDataPoint p;
                    std::vector<std::string> values = splitString(data, '&');
                    std::string dateStr = path.substr(1);
                    std::vector<std::string> dateParts = splitString(dateStr, '-');

                    if (dateParts.size() != 3)
                        throw std::exception();

                    p.date = Date(stoi(dateParts[0]), stoi(dateParts[1]), stoi(dateParts[2]));

                    for (std::string value : values)
                    {
                        std::vector<std::string> pair = splitString(value, '=');
                        if (pair[0] == "weather_code")
                        {
                            p.weather_code = stoi(pair[1]);
                        }
                        else if (pair[0] == "temperature_max")
                        {
                            p.temperature_max = stod(pair[1]);
                        }
                        else if (pair[0] == "temperature_min")
                        {
                            p.temperature_min = stod(pair[1]);
                        }
                        else if (pair[0] == "precipitation_sum")
                        {
                            p.precipitation_sum = stod(pair[1]);
                        }
                        else if (pair[0] == "wind_speed_max")
                        {
                            p.wind_speed_max = stod(pair[1]);
                        }
                        else if (pair[0] == "precipitation_probability_max")
                        {
                            p.precipitation_probability_max = stoi(pair[1]);
                        }
                    }

                    // Insert data (if data point already exists, request is invalid)
                    if (!this->weatherData->insert(std::pair<Date, WeatherDataPoint>(p.date, p)).second)
                        throw std::exception();

                    std::string response = "HTTP/1.1 204 No Content\r\nContent-Length: 0";
                    send(newSocket, response.c_str(), response.size(), 0);
                }
                catch (const std::exception &e)
                {
                    std::string response = "HTTP/1.1 400 Bad Request\r\nContent-Length: 0";
                    send(newSocket, response.c_str(), response.size(), 0);
                }
            }
            else if (method == "PUT")
            {
                try
                {
                    if (path.size() <= 1) // No date specified
                        throw std::exception();

                    // Parse date from path
                    std::string dateStr = path.substr(1);
                    std::vector<std::string> dateParts = splitString(dateStr, '-');
                    if (dateParts.size() != 3)
                        throw std::exception();

                    Date date = Date(stoi(dateParts[0]), stoi(dateParts[1]), stoi(dateParts[2]));
                    if (this->weatherData->find(date) == this->weatherData->end())
                        throw std::exception();

                    // Check replacement value and apply
                    std::vector<std::string> valuePair = splitString(data, '=');
                    
                    if (valuePair[0] == "weather_code")
                        this->weatherData->at(date).weather_code = stoi(valuePair[1]);
                    else if (valuePair[0] == "temperature_max")
                        this->weatherData->at(date).temperature_max = stod(valuePair[1]);
                    else if (valuePair[0] == "temperature_min")
                        this->weatherData->at(date).temperature_min = stod(valuePair[1]);
                    else if (valuePair[0] == "precipitation_sum")
                        this->weatherData->at(date).precipitation_sum = stod(valuePair[1]);
                    else if (valuePair[0] == "wind_speed_max")
                        this->weatherData->at(date).wind_speed_max = stod(valuePair[1]);
                    else if (valuePair[0] == "precipitation_probability_max")
                        this->weatherData->at(date).precipitation_probability_max = stoi(valuePair[1]);
                    else
                        throw std::exception();

                    std::string response = "HTTP/1.1 204 No Content\r\nContent-Length: 0";
                    send(newSocket, response.c_str(), response.size(), 0);
                }
                catch (const std::exception &e)
                {
                    std::string response = "HTTP/1.1 400 Bad Request\r\nContent-Length: 0";
                    send(newSocket, response.c_str(), response.size(), 0);
                }
            }
            else if (method == "DELETE")
            {
                try
                {
                    if (path.size() <= 1) // No date specified
                        throw std::exception();

                    // Extract date from path
                    std::vector<std::string> dateParts = splitString(path.substr(1), '-');
                    if (dateParts.size() != 3)
                        throw std::exception();

                    Date date = Date(stoi(dateParts[0]), stoi(dateParts[1]), stoi(dateParts[2]));

                    // If date corresponds to valid data, delete that data
                    auto pos = this->weatherData->find(date);
                    if (pos == this->weatherData->end())
                        throw std::exception();

                    this->weatherData->erase(pos);
                    std::string response = "HTTP/1.1 204 No Content\r\nContent-Length: 0";
                    send(newSocket, response.c_str(), response.size(), 0);
                }
                catch (const std::exception &e)
                {
                    std::string response = "HTTP/1.1 400 Bad Request\r\nContent-Length: 0";
                    send(newSocket, response.c_str(), response.size(), 0);
                }
            }

            // Finished with request, reopen data access
            *this->weatherDataLock = false;
            
            close(newSocket);
        }
    }

    close(serverFd);
}