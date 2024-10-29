#include "weatherapi.hpp"

WeatherDataPoint WeatherDataAPIHandler::GetData(double latitude, double longitude, Date date)
{
    // Get API data
    std::string jsonString = GetJSONFromAPI(latitude, longitude, date);

    try
    {
        // Process data from JSON
        WeatherDataPoint result;

        auto data = nlohmann::json::parse(jsonString);

        result.date = date;

        result.weather_code = data["daily"]["weather_code"].get<std::vector<int> >().at(0);
        result.temperature_max = data["daily"]["temperature_2m_max"].get<std::vector<double> >().at(0);
        result.temperature_min = data["daily"]["temperature_2m_min"].get<std::vector<double> >().at(0);
        result.precipitation_sum = data["daily"]["precipitation_sum"].get<std::vector<double> >().at(0);
        result.precipitation_probability_max = -1;
        result.wind_speed_max = data["daily"]["wind_speed_10m_max"].get<std::vector<double> >().at(0);

        return result;
    }
    catch (const std::out_of_range &e)
    {
        return WeatherDataPoint();
    }
}

std::vector<WeatherDataPoint> WeatherDataAPIHandler::GetData(double latitude, double longitude, Date startDate, Date endDate)
{
    // Get API data
    std::string jsonString = GetJSONFromAPI(latitude, longitude, startDate, endDate);

    try
    {
        // Process data from JSON
        auto data = nlohmann::json::parse(jsonString);

        auto dates = data["daily"]["time"].get<std::vector<std::string> >();
        auto weatherCodes = data["daily"]["weather_code"].get<std::vector<int> >();
        auto maxTemps = data["daily"]["temperature_2m_max"].get<std::vector<double> >();
        auto minTemps = data["daily"]["temperature_2m_min"].get<std::vector<double> >();
        auto precipitationSums = data["daily"]["precipitation_sum"].get<std::vector<double> >();
        auto windSpeedMaxes = data["daily"]["wind_speed_10m_max"].get<std::vector<double> >();

        std::vector<WeatherDataPoint> result;
        for (unsigned int i = 0; i < dates.size(); i++)
        {
            WeatherDataPoint newPoint;

            auto date = dates[i];
            auto dateParts = splitString(date, '-');
            newPoint.date = Date(stoi(dateParts[0]), stoi(dateParts[1]), stoi(dateParts[2]));

            newPoint.weather_code = weatherCodes.at(i);
            newPoint.temperature_max = maxTemps.at(i);
            newPoint.temperature_min = minTemps.at(i);
            newPoint.precipitation_sum = precipitationSums.at(i);
            newPoint.precipitation_probability_max = -1;
            newPoint.wind_speed_max = windSpeedMaxes.at(i);

            result.push_back(newPoint);
        }

        return result;
    }
    catch (const std::exception &e)
    {
        return std::vector<WeatherDataPoint>();
    }
}

size_t WeatherDataAPIHandler::HTTPResultWrite(void* ptr, size_t size, size_t nmemb, std::string* s)
{
    size_t newLen = size * nmemb;
    
    s->append((char*)ptr, newLen);
    
    return newLen;
}

std::string WeatherDataAPIHandler::GetJSONFromAPI(double latitude, double longitude, Date date)
{
    // Get URL for HTTP request
    std::stringstream s;
    s << "https://historical-forecast-api.open-meteo.com/v1/forecast?latitude=" << latitude << "&longitude=" << longitude;
    s << "&daily=weather_code,temperature_2m_max,temperature_2m_min,precipitation_sum,wind_speed_10m_max&temperature_unit=fahrenheit&wind_speed_unit=mph&precipitation_unit=inch";
    s << "&start_date=" << std::setfill('0') << std::setw(4) << date.year << std::setw(0) << '-' << std::setw(2) << date.month << std::setw(0) << '-' << std::setw(2) << date.day << std::setw(0);
    s << "&end_date=" << std::setfill('0') << std::setw(4) << date.year << std::setw(0) << '-' << std::setw(2) << date.month << std::setw(0) << '-' << std::setw(2) << date.day << std::setw(0);

    auto handle = curl_easy_init();
    if (!handle)
    {
        std::cout << "CURL failed to init!" << std::endl;
        return "";
    }

    std::string data;
    curl_easy_setopt(handle, CURLOPT_CAINFO, (DATA_DIR + "cacert.pem").c_str());
    curl_easy_setopt(handle, CURLOPT_URL, s.str().c_str());
    curl_easy_setopt(handle, CURLOPT_HTTPGET, 1L);
    curl_easy_setopt(handle, CURLOPT_WRITEFUNCTION, HTTPResultWrite);
    curl_easy_setopt(handle, CURLOPT_WRITEDATA, &data);

    auto res = curl_easy_perform(handle);
    curl_easy_cleanup(handle);

    if (res != CURLE_OK)
    {
        std::cout << "curl_easy_perform() failed: " << curl_easy_strerror(res) << std::endl;
        return curl_easy_strerror(res);
    }

    return data;
}

std::string WeatherDataAPIHandler::GetJSONFromAPI(double latitude, double longitude, Date startDate, Date endDate)
{
    // Get URL for HTTP request
    std::stringstream s;
    s << "https://historical-forecast-api.open-meteo.com/v1/forecast?latitude=" << latitude << "&longitude=" << longitude;
    s << "&daily=weather_code,temperature_2m_max,temperature_2m_min,precipitation_sum,wind_speed_10m_max&temperature_unit=fahrenheit&wind_speed_unit=mph&precipitation_unit=inch";
    s << "&start_date=" << std::setfill('0') << std::setw(4) << startDate.year << std::setw(0) << '-' << std::setw(2) << startDate.month << std::setw(0) << '-' << std::setw(2) << startDate.day << std::setw(0);
    s << "&end_date=" << std::setfill('0') << std::setw(4) << endDate.year << std::setw(0) << '-' << std::setw(2) << endDate.month << std::setw(0) << '-' << std::setw(2) << endDate.day << std::setw(0);

    auto handle = curl_easy_init();
    if (!handle)
    {
        std::cout << "CURL failed to init!" << std::endl;
        return "";
    }

    std::string data;
    curl_easy_setopt(handle, CURLOPT_CAINFO, (DATA_DIR + "cacert.pem").c_str());
    curl_easy_setopt(handle, CURLOPT_URL, s.str().c_str());
    curl_easy_setopt(handle, CURLOPT_HTTPGET, 1L);
    curl_easy_setopt(handle, CURLOPT_WRITEFUNCTION, HTTPResultWrite);
    curl_easy_setopt(handle, CURLOPT_WRITEDATA, &data);

    auto res = curl_easy_perform(handle);
    curl_easy_cleanup(handle);

    if (res != CURLE_OK)
    {
        std::cout << "curl_easy_perform() failed: " << curl_easy_strerror(res) << std::endl;
        return "";
    }

    return data;
}