#pragma once

#include "global.hpp"
#include "datapoint.hpp"
#include "ui/input.hpp"
#include "ui/scene.hpp"
#include "ui/text.hpp"
#include "ui/button.hpp"
#include "ui/textinput.hpp"
#include "ui/histogram.hpp"
#include "weatherapi.hpp"
#include "httpserver.hpp"

class Core
{
    public:
        Core();
        ~Core();
        void Update();
        void Render();

        float programRunTime = 0.0f;
        bool weatherDataLock = false;
    private:
        C3D_RenderTarget *topLeft, *topRight, *bottom;
        Scene currentScene;
        
        std::map<Date, WeatherDataPoint> weatherData;

        Input input;
        HTTPServer httpServer;

        Thread serverThread;

        // Multiple data points
        static void SetDataViewText(std::map<Date, WeatherDataPoint> &data, std::string dataType, std::string dataTypeName, std::string unit, std::string comparison, Scene page)
        {
            std::string startDate = ((TextInput*)page.bottomScreenObjects[0].get())->GetCurrentText();
            std::string endDate = ((TextInput*)page.bottomScreenObjects[1].get())->GetCurrentText();

            // Get included data points from inputs
            auto dataAsVector = weatherDataMapToVector(data);
            auto includedPoints = getDataFromRangeStrings(dataAsVector, startDate, endDate);

            // Check if input was valid
            if (includedPoints.size() == 0)
            {
                auto text = (Text*)page.topScreenObjects[0].get();
                text->SetText("No valid data found in provided bounds");
                text->color = Color::RED;
                return;
            }

            double val = 0.0;
            if (comparison == "minimum")
                val = getMinOfPoint(includedPoints, dataType);
            else if (comparison == "maximum")
                val = getMaxOfPoint(includedPoints, dataType);
            else if (comparison == "average")
                val = getAverageOfPoint(includedPoints, dataType);

            // Assemble message
            std::stringstream s;
            s << "The " << comparison << ' ' << dataTypeName << " from date " << startDate << " to date " << endDate << " is " << val << unit << '.';

            // Display message
            auto text = (Text*)page.topScreenObjects[0].get();
            text->color = Color::BLACK;
            text->SetText(s.str());
        }

        // Single data point
        static void SetDataViewText(std::map<Date, WeatherDataPoint> &data, std::string dataType, std::string dataTypeName, std::string unit, Scene page)
        {
            std::string date = ((TextInput*)page.bottomScreenObjects[0].get())->GetCurrentText();

            // Get min from inputs
            auto dataAsVector = weatherDataMapToVector(data);
            auto includedPoints = getDataFromRangeStrings(dataAsVector, date, date);

            // Check if input was valid
            if (includedPoints.size() == 0)
            {
                auto text = (Text*)page.topScreenObjects[0].get();
                text->SetText("No valid data found for provided date");
                text->color = Color::RED;
                return;
            }

            double val = 0.0;
            if (dataType == "weather_code")
                val = includedPoints[0].weather_code;
            else if (dataType == "temperature_max")
                val = includedPoints[0].temperature_max;
            else if (dataType == "temperature_min")
                val = includedPoints[0].temperature_min;
            else if (dataType == "precipitation_sum")
                val = includedPoints[0].precipitation_sum;
            else if (dataType == "wind_speed_max")
                val = includedPoints[0].wind_speed_max;
            else if (dataType == "precipitation_probability_max")
                val = includedPoints[0].precipitation_probability_max;

            // Assemble message
            std::stringstream s;
            s << "The " << dataTypeName << " on date " << date << " is " << val << unit << '.';

            // Display message
            auto text = (Text*)page.topScreenObjects[0].get();
            text->color = Color::BLACK;
            text->SetText(s.str());
        }

        // Multiple data points for API
        static void SetDataViewTextWithAPI(std::map<Date, WeatherDataPoint> &data, std::string dataType, std::string dataTypeName, std::string unit, std::string comparison, Scene page)
        {
            std::string startDateStr = ((TextInput*)page.bottomScreenObjects[0].get())->GetCurrentText();
            std::string endDateStr = ((TextInput*)page.bottomScreenObjects[1].get())->GetCurrentText();
            std::string latitudeStr = ((TextInput*)page.bottomScreenObjects[2].get())->GetCurrentText();
            std::string longitudeStr = ((TextInput*)page.bottomScreenObjects[3].get())->GetCurrentText();

            double latitude, longitude;
            try
            {
                latitude = stod(latitudeStr);
                longitude = stod(longitudeStr);
            }
            catch(const std::invalid_argument& e)
            {
                auto text = (Text*)page.topScreenObjects[0].get();
                text->SetText("Invalid location");
                text->color = Color::RED;
                return;
            }
            

            // Get included data points from inputs
            auto dataAsVector = weatherDataMapToVector(data);
            auto includedPoints = getDataFromRangeStrings(dataAsVector, startDateStr, endDateStr);

            // Check if input was valid
            if (includedPoints.size() == 0)
            {
                auto text = (Text*)page.topScreenObjects[0].get();
                text->SetText("No valid data found in provided bounds");
                text->color = Color::RED;
                return;
            }

            // Get local data
            double localVal = 0.0;
            if (comparison == "minimum")
                localVal = getMinOfPoint(includedPoints, dataType);
            else if (comparison == "maximum")
                localVal = getMaxOfPoint(includedPoints, dataType);
            else if (comparison == "average")
                localVal = getAverageOfPoint(includedPoints, dataType);

            // Get API data

            // Get start date
            Date start;
            std::vector<std::string> startDateParts = splitString(startDateStr, '-');
            if (startDateParts.size() == 1)
            {
                start = dataAsVector[stoi(startDateStr)].date;
            }
            else
            {
                start = Date(stoi(startDateParts[0]), stoi(startDateParts[1]), stoi(startDateParts[2]));
            }
            
            // Get end date
            Date end;
            std::vector<std::string> endDateParts = splitString(endDateStr, '-');
            if (endDateParts.size() == 1)
            {
                end = dataAsVector[stoi(endDateStr)].date;
            }
            else
            {
                end = Date(stoi(endDateParts[0]), stoi(endDateParts[1]), stoi(endDateParts[2]));
            }

            // Retrieve weather data from API
            auto apiPoints = WeatherDataAPIHandler::GetData(latitude, longitude, start, end);

            double apiVal = 0.0;
            if (comparison == "minimum")
                apiVal = getMinOfPoint(apiPoints, dataType);
            else if (comparison == "maximum")
                apiVal = getMaxOfPoint(apiPoints, dataType);
            else if (comparison == "average")
                apiVal = getAverageOfPoint(apiPoints, dataType);

            // Assemble message
            std::stringstream s;
            s << "The " << comparison << ' ' << dataTypeName << " from date " << startDateStr << " to date " << endDateStr << " was " << localVal << unit << ", while at latitude " << latitude << " and longitude " << longitude << ", it was " << apiVal << unit << '.';

            // Display message
            auto text = (Text*)page.topScreenObjects[0].get();
            text->color = Color::BLACK;
            text->SetText(s.str());
        }

        // Single data point for API
        static void SetDataViewTextWithAPI(std::map<Date, WeatherDataPoint> &data, std::string dataType, std::string dataTypeName, std::string unit, Scene page)
        {
            std::string dateStr = ((TextInput*)page.bottomScreenObjects[0].get())->GetCurrentText();
            std::string latitudeStr = ((TextInput*)page.bottomScreenObjects[1].get())->GetCurrentText();
            std::string longitudeStr = ((TextInput*)page.bottomScreenObjects[2].get())->GetCurrentText();

            double latitude, longitude;
            try
            {
                latitude = stod(latitudeStr);
                longitude = stod(longitudeStr);
            }
            catch(const std::invalid_argument& e)
            {
                auto text = (Text*)page.topScreenObjects[0].get();
                text->SetText("Invalid location");
                text->color = Color::RED;
                return;
            }

            // Get included data points from inputs
            auto dataAsVector = weatherDataMapToVector(data);
            auto includedPoints = getDataFromRangeStrings(dataAsVector, dateStr, dateStr);

            // Check if input was valid
            if (includedPoints.size() == 0)
            {
                auto text = (Text*)page.topScreenObjects[0].get();
                text->SetText("No valid data found in provided bounds");
                text->color = Color::RED;
                return;
            }

            // Get local data
            double localVal = 0.0;
            if (dataType == "weather_code")
                localVal = includedPoints[0].weather_code;
            else if (dataType == "temperature_max")
                localVal = includedPoints[0].temperature_max;
            else if (dataType == "temperature_min")
                localVal = includedPoints[0].temperature_min;
            else if (dataType == "precipitation_sum")
                localVal = includedPoints[0].precipitation_sum;
            else if (dataType == "wind_speed_max")
                localVal = includedPoints[0].wind_speed_max;
            else if (dataType == "precipitation_probability_max")
                localVal = includedPoints[0].precipitation_probability_max;

            // Get API data

            // Get date
            Date date;
            std::vector<std::string> dateParts = splitString(dateStr, '-');
            if (dateParts.size() == 1)
            {
                date = dataAsVector[stoi(dateStr)].date;
            }
            else
            {
                date = Date(stoi(dateParts[0]), stoi(dateParts[1]), stoi(dateParts[2]));
            }

            // Retrieve weather data from API
            auto apiPoint = WeatherDataAPIHandler::GetData(latitude, longitude, date);

            double apiVal = 0.0;
            if (dataType == "weather_code")
                apiVal = apiPoint.weather_code;
            else if (dataType == "temperature_max")
                apiVal = apiPoint.temperature_max;
            else if (dataType == "temperature_min")
                apiVal = apiPoint.temperature_min;
            else if (dataType == "precipitation_sum")
                apiVal = apiPoint.precipitation_sum;
            else if (dataType == "wind_speed_max")
                apiVal = apiPoint.wind_speed_max;
            else if (dataType == "precipitation_probability_max")
                apiVal = apiPoint.precipitation_probability_max;

            // Assemble message
            std::stringstream s;
            s << "The " << dataTypeName << " on date " << dateStr << " was " << localVal << unit << ", while at latitude " << latitude << " and longitude " << longitude << ", it was " << apiVal << unit << '.';

            // Display message
            auto text = (Text*)page.topScreenObjects[0].get();
            text->color = Color::BLACK;
            text->SetText(s.str());
        }

        
        // Histogram creation
        static void CreateHistogram(std::map<Date, WeatherDataPoint> &data, std::string dataType, std::string dataTypeName, std::string unit, Scene &page)
        {
            // Get rid of any existing objects
            page.topScreenObjects.clear();
            
            std::string startDate = ((TextInput*)page.bottomScreenObjects[0].get())->GetCurrentText();
            std::string endDate = ((TextInput*)page.bottomScreenObjects[1].get())->GetCurrentText();

            // Get included data points from inputs
            auto dataAsVector = weatherDataMapToVector(data);
            auto includedPoints = getDataFromRangeStrings(dataAsVector, startDate, endDate);

            // Check if input was valid
            if (includedPoints.size() == 0)
            {
                page.topScreenObjects.push_back(std::shared_ptr<Text>(new Text("No valid data in provided bounds", 200, 20, 1.0f, 360, 1.0f, 1.0f, Color::RED, C2D_WithColor | C2D_AlignCenter | C2D_WordWrap)));
                return;
            }
            
            // Create histogram
            page.topScreenObjects.push_back(std::shared_ptr<Histogram>(new Histogram(includedPoints, dataType, 6, 6,
                  dataTypeName, "Instances", Color::BLACK, Color::GREEN, Color::BLACK, 50, 10, 0.5f, 300, 200)));
        }

        // Scenes
        // keep in mind, stuff is drawn in order (make sure things further back are earlier in the list)

        Scene baseScenePage1 = 
        { 
            // Name
            "Base Scene Page 1",

            // Top & bottom background color
            Color::GREY, Color::SILVER,

            // Top screen
            { 
                std::shared_ptr<Text>(new Text("WEATHER SIMULATOR 9000", 200, 20, 2.0f, 360, 2.0f, 2.0f, Color::MAROON, C2D_WithColor | C2D_AlignCenter | C2D_WordWrap)),
                std::shared_ptr<Text>(new Text("wowza!! ", 260, 180, 0.0f, 360, 1.15f, 1.15f, Color::YELLOW, C2D_WithColor | C2D_AlignCenter)),
            },

            // Bottom screen
            { 
                std::shared_ptr<Button>(new Button([this]() { this->currentScene = this->loadWeatherDataPage; }, Color::WHITE, Color::BLUE, "Load Data", Color::BLACK, Color::WHITE, 55, 30, 0, 200, 42, 0.65f, 0.65f)),
                std::shared_ptr<Button>(new Button([this]() { this->currentScene = this->viewWeatherDataPage; }, Color::WHITE, Color::BLUE, "View Data", Color::BLACK, Color::WHITE, 55, 82, 0, 200, 42, 0.65f, 0.65f)),
                std::shared_ptr<Button>(new Button([this]() { this->currentScene = this->compareAPIDataPage; }, Color::WHITE, Color::BLUE, "Compare Data With API", Color::BLACK, Color::WHITE, 55, 134, 0, 200, 42, 0.65f, 0.65f)),
                std::shared_ptr<Button>(new Button([this]() { this->currentScene = this->baseScenePage2; }, Color::WHITE, Color::BLUE, "Next Page", Color::BLACK, Color::WHITE, 246, 200, 0, 64, 32, 0.5f, 0.5f)), 
            },

            // Update func
            std::function<void()>([this]() 
            {
                // Move text around on title
                Text* titleText = (Text*)this->baseScenePage1.topScreenObjects[0].get();
                titleText->y = 20.0f + sinf(programRunTime * 0.002f) * 7.5f;

                Text* flavorText = (Text*)this->baseScenePage1.topScreenObjects[1].get();
                flavorText->scaleX = 1.15f + sinf(programRunTime * 0.007f) * 0.125f;
                flavorText->scaleY = 1.15f + sinf((programRunTime * 0.007f) + 10.0f) * 0.05f;
                flavorText->z = sinf(programRunTime * 0.007f) * 0.65f;
            } )
        };

        Scene baseScenePage2 = 
        { 
            // Name
            "Base Scene Page 2",

            // Background color
            Color::GREY, Color::SILVER,

            // Top screen (same as above)
            this->baseScenePage1.topScreenObjects,

            // Bottom screen
            { 
                std::shared_ptr<Button>(new Button([this]() { this->currentScene = this->createHistogramPage; }, Color::WHITE, Color::BLUE, "Create Histogram", Color::BLACK, Color::WHITE, 55, 30, 0, 200, 42, 0.65f, 0.65f)),
                std::shared_ptr<Button>(new Button([this]() { this->currentScene = this->baseScenePage1; }, Color::WHITE, Color::BLUE, "Prev Page", Color::BLACK, Color::WHITE, 10, 200, 0, 64, 32, 0.5f, 0.5f)), 
            },

            // Update func (same as above)
            this->baseScenePage1.Update
        };

        Scene loadWeatherDataPage = 
        { 
            // Name
            "Load Weather Data Page",

            // Background color
            Color::GREY, Color::SILVER,

            // Top screen
            this->baseScenePage1.topScreenObjects,

            // Bottom screen
            { 
                std::shared_ptr<TextInput>(new TextInput(Color::WHITE, "Enter filename...", Color::BLACK, MAX_INPUT_SIZE, SWKBD_TYPE_QWERTY, 40, 30, 0, 240, 36, 0.65f, 0.65f)),

                std::shared_ptr<Button>(new Button([this]() 
                { 
                    // Load weather data from filename in text box
                    while (this->weatherDataLock) {}

                    this->weatherDataLock = true;

                    this->weatherData.clear();
                    WeatherDataPoint::readFromFile(((TextInput*)this->loadWeatherDataPage.bottomScreenObjects[0].get())->GetCurrentText(), &this->weatherData);
                    if (this->weatherData.size() != 0)
                        this->currentScene = this->baseScenePage1;
                    // If load fails, report incorrect filename
                    else if (this->currentScene.topScreenObjects.size() == 0)
                        this->currentScene.topScreenObjects.push_back(std::shared_ptr<Text>(new Text("Invalid filename...I'm so sorry, dude...no, dude, please, don't cry, it's okay bro, just try another filename or something...no, dude, bro, i swear it's cool, don't sweat it, just choose another file and i promise it will be okay", 200, 50, 3.0f, 360, 0.9f, 0.9f, Color::RED, C2D_WithColor | C2D_AlignCenter | C2D_WordWrap)));

                    this->weatherDataLock = false;
                }, 
                Color::WHITE, Color::BLUE, "Load File", Color::BLACK, Color::WHITE, 55, 96, 0, 200, 42, 0.65f, 0.65f)),

                std::shared_ptr<Button>(new Button([this]() { this->currentScene = this->baseScenePage1; }, Color::WHITE, Color::BLUE, "Back ", Color::BLACK, Color::WHITE, 10, 200, 0, 64, 32, 0.5f, 0.5f)), 
            },

            // Update func
            this->baseScenePage1.Update
        };

        // View data pages
        Scene viewWeatherDataPage =
        { 
            // Name
            "View Weather Data Page",

            // Background color
            Color::GREY, Color::SILVER,

            // Top screen
            this->baseScenePage1.topScreenObjects,

            // Bottom screen
            { 
                std::shared_ptr<Button>(new Button([this]() { this->currentScene = this->viewWeatherDataMinPage; }, Color::WHITE, Color::BLUE, "Minimum ", Color::BLACK, Color::WHITE, 25, 60, 0, 125, 42, 0.65f, 0.65f)),
                std::shared_ptr<Button>(new Button([this]() { this->currentScene = this->viewWeatherDataMaxPage; }, Color::WHITE, Color::BLUE, "Maximum ", Color::BLACK, Color::WHITE, 170, 60, 0, 125, 42, 0.65f, 0.65f)),
                std::shared_ptr<Button>(new Button([this]() { this->currentScene = this->viewWeatherDataAveragePage; }, Color::WHITE, Color::BLUE, "Average ", Color::BLACK, Color::WHITE, 25, 112, 0, 125, 42, 0.65f, 0.65f)),
                std::shared_ptr<Button>(new Button([this]() { this->currentScene = this->viewWeatherDataSinglePointPage; }, Color::WHITE, Color::BLUE, "Single Point", Color::BLACK, Color::WHITE, 170, 112, 0, 125, 42, 0.65f, 0.65f)),
                std::shared_ptr<Button>(new Button([this]() { this->currentScene = this->baseScenePage1; }, Color::WHITE, Color::BLUE, "Back ", Color::BLACK, Color::WHITE, 10, 200, 0, 64, 32, 0.5f, 0.5f)), 
            },

            // Update func
            this->baseScenePage1.Update
        };

        Scene viewWeatherDataMinPage =
        { 
            // Name
            "View Weather Data Min Page",

            // Background color
            Color::GREY, Color::SILVER,

            // Top screen
            {
                std::shared_ptr<Text>(new Text(" ", 200, 20, 1.0f, 360, 1.1f, 1.1f, Color::BLACK, C2D_WithColor | C2D_AlignCenter | C2D_WordWrap)),
            },

            // Bottom screen
            { 
                std::shared_ptr<TextInput>(new TextInput(Color::WHITE, "Start date...", Color::BLACK, MAX_INPUT_SIZE, SWKBD_TYPE_NORMAL, 20, 20, 0, 125, 32, 0.65f, 0.65f)),
                std::shared_ptr<TextInput>(new TextInput(Color::WHITE, "End date...", Color::BLACK, MAX_INPUT_SIZE, SWKBD_TYPE_NORMAL, 175, 20, 0, 125, 32, 0.65f, 0.65f)),
                std::shared_ptr<Button>(new Button([this]() { 
                    SetDataViewText(this->weatherData, "weather_code", "weather code", "", "minimum", this->currentScene);
                }, Color::WHITE, Color::BLUE, "Weather\nCode", Color::BLACK, Color::WHITE, 10, 60, 0, 90, 56, 0.6f, 0.6f)),
                std::shared_ptr<Button>(new Button([this]() {
                    SetDataViewText(this->weatherData, "temperature_max", "temperature max", " °F", "minimum", this->currentScene);
                }, Color::WHITE, Color::BLUE, "Temp Max", Color::BLACK, Color::WHITE, 115, 60, 0, 90, 56, 0.6f, 0.6f)),
                std::shared_ptr<Button>(new Button([this]() {
                    SetDataViewText(this->weatherData, "temperature_min", "temperature min", " °F", "minimum", this->currentScene);
                }, Color::WHITE, Color::BLUE, "Temp Min", Color::BLACK, Color::WHITE, 220, 60, 0, 90, 56, 0.6f, 0.6f)),
                std::shared_ptr<Button>(new Button([this]() {
                    SetDataViewText(this->weatherData, "precipitation_sum", "precipitation sum", "\"", "minimum", this->currentScene);
                }, Color::WHITE, Color::BLUE, "Precip Sum", Color::BLACK, Color::WHITE, 10, 130, 0, 90, 56, 0.6f, 0.6f)),
                std::shared_ptr<Button>(new Button([this]() {
                    SetDataViewText(this->weatherData, "wind_speed_max", "wind speed max", " mph", "minimum", this->currentScene);
                }, Color::WHITE, Color::BLUE, "Wind Speed\nMax", Color::BLACK, Color::WHITE, 115, 130, 0, 90, 56, 0.6f, 0.6f)),
                std::shared_ptr<Button>(new Button([this]() {
                    SetDataViewText(this->weatherData, "precipitation_probability_max", "precipitation probability max", "%", "minimum", this->currentScene);
                }, Color::WHITE, Color::BLUE, "Precip Prob\nMax", Color::BLACK, Color::WHITE, 220, 130, 0, 90, 56, 0.6f, 0.6f)),
                std::shared_ptr<Button>(new Button([this]() { 
                    // Reset fields
                    auto text = (Text*)this->currentScene.topScreenObjects[0].get();
                    text->SetText(" ");
                    text->color = Color::BLACK;

                    ((TextInput*)this->currentScene.bottomScreenObjects[0].get())->Reset();
                    ((TextInput*)this->currentScene.bottomScreenObjects[1].get())->Reset();

                    // Go back
                    this->currentScene = this->viewWeatherDataPage;
                }, Color::WHITE, Color::BLUE, "Back ", Color::BLACK, Color::WHITE, 10, 200, 0, 64, 32, 0.5f, 0.5f)), 
            },

            // Update func
            this->baseScenePage1.Update
        };

        Scene viewWeatherDataMaxPage =
        { 
            // Name
            "View Weather Data Max Page",

            // Background color
            Color::GREY, Color::SILVER,

            // Top screen
            {
                std::shared_ptr<Text>(new Text(" ", 200, 20, 1.0f, 360, 1.1f, 1.1f, Color::BLACK, C2D_WithColor | C2D_AlignCenter | C2D_WordWrap)),
            },

            // Bottom screen
            { 
                std::shared_ptr<TextInput>(new TextInput(Color::WHITE, "Start date...", Color::BLACK, MAX_INPUT_SIZE, SWKBD_TYPE_NORMAL, 20, 20, 0, 125, 32, 0.65f, 0.65f)),
                std::shared_ptr<TextInput>(new TextInput(Color::WHITE, "End date...", Color::BLACK, MAX_INPUT_SIZE, SWKBD_TYPE_NORMAL, 175, 20, 0, 125, 32, 0.65f, 0.65f)),
                std::shared_ptr<Button>(new Button([this]() { 
                    SetDataViewText(this->weatherData, "weather_code", "weather code", "", "maximum", this->currentScene);
                }, Color::WHITE, Color::BLUE, "Weather\nCode", Color::BLACK, Color::WHITE, 10, 60, 0, 90, 56, 0.6f, 0.6f)),
                std::shared_ptr<Button>(new Button([this]() {
                    SetDataViewText(this->weatherData, "temperature_max", "temperature max", " °F", "maximum", this->currentScene);
                }, Color::WHITE, Color::BLUE, "Temp Max", Color::BLACK, Color::WHITE, 115, 60, 0, 90, 56, 0.6f, 0.6f)),
                std::shared_ptr<Button>(new Button([this]() {
                    SetDataViewText(this->weatherData, "temperature_min", "temperature min", " °F", "maximum", this->currentScene);
                }, Color::WHITE, Color::BLUE, "Temp Min", Color::BLACK, Color::WHITE, 220, 60, 0, 90, 56, 0.6f, 0.6f)),
                std::shared_ptr<Button>(new Button([this]() {
                    SetDataViewText(this->weatherData, "precipitation_sum", "precipitation sum", "\"", "maximum", this->currentScene);
                }, Color::WHITE, Color::BLUE, "Precip Sum", Color::BLACK, Color::WHITE, 10, 130, 0, 90, 56, 0.6f, 0.6f)),
                std::shared_ptr<Button>(new Button([this]() {
                    SetDataViewText(this->weatherData, "wind_speed_max", "wind speed max", " mph", "maximum", this->currentScene);
                }, Color::WHITE, Color::BLUE, "Wind Speed\nMax", Color::BLACK, Color::WHITE, 115, 130, 0, 90, 56, 0.6f, 0.6f)),
                std::shared_ptr<Button>(new Button([this]() {
                    SetDataViewText(this->weatherData, "precipitation_probability_max", "precipitation probability max", "%", "maximum", this->currentScene);
                }, Color::WHITE, Color::BLUE, "Precip Prob\nMax", Color::BLACK, Color::WHITE, 220, 130, 0, 90, 56, 0.6f, 0.6f)),
                std::shared_ptr<Button>(new Button([this]() { 
                    // Reset fields
                    auto text = (Text*)this->currentScene.topScreenObjects[0].get();
                    text->SetText(" ");
                    text->color = Color::BLACK;

                    ((TextInput*)this->currentScene.bottomScreenObjects[0].get())->Reset();
                    ((TextInput*)this->currentScene.bottomScreenObjects[1].get())->Reset();

                    // Go back
                    this->currentScene = this->viewWeatherDataPage;
                }, Color::WHITE, Color::BLUE, "Back ", Color::BLACK, Color::WHITE, 10, 200, 0, 64, 32, 0.5f, 0.5f)), 
            },

            // Update func
            this->baseScenePage1.Update
        };

        Scene viewWeatherDataAveragePage =
        { 
            // Name
            "View Weather Data Average Page",

            // Background color
            Color::GREY, Color::SILVER,

            // Top screen
            {
                std::shared_ptr<Text>(new Text(" ", 200, 20, 1.0f, 360, 1.1f, 1.1f, Color::BLACK, C2D_WithColor | C2D_AlignCenter | C2D_WordWrap)),
            },

            // Bottom screen
            { 
                std::shared_ptr<TextInput>(new TextInput(Color::WHITE, "Start date...", Color::BLACK, MAX_INPUT_SIZE, SWKBD_TYPE_NORMAL, 20, 20, 0, 125, 32, 0.65f, 0.65f)),
                std::shared_ptr<TextInput>(new TextInput(Color::WHITE, "End date...", Color::BLACK, MAX_INPUT_SIZE, SWKBD_TYPE_NORMAL, 175, 20, 0, 125, 32, 0.65f, 0.65f)),
                std::shared_ptr<Button>(new Button([this]() { 
                    SetDataViewText(this->weatherData, "weather_code", "weather code", "", "average", this->currentScene);
                }, Color::WHITE, Color::BLUE, "Weather\nCode", Color::BLACK, Color::WHITE, 10, 60, 0, 90, 56, 0.6f, 0.6f)),
                std::shared_ptr<Button>(new Button([this]() {
                    SetDataViewText(this->weatherData, "temperature_max", "temperature max", " °F", "average", this->currentScene);
                }, Color::WHITE, Color::BLUE, "Temp Max", Color::BLACK, Color::WHITE, 115, 60, 0, 90, 56, 0.6f, 0.6f)),
                std::shared_ptr<Button>(new Button([this]() {
                    SetDataViewText(this->weatherData, "temperature_min", "temperature min", " °F", "average", this->currentScene);
                }, Color::WHITE, Color::BLUE, "Temp Min", Color::BLACK, Color::WHITE, 220, 60, 0, 90, 56, 0.6f, 0.6f)),
                std::shared_ptr<Button>(new Button([this]() {
                    SetDataViewText(this->weatherData, "precipitation_sum", "precipitation sum", "\"", "average", this->currentScene);
                }, Color::WHITE, Color::BLUE, "Precip Sum", Color::BLACK, Color::WHITE, 10, 130, 0, 90, 56, 0.6f, 0.6f)),
                std::shared_ptr<Button>(new Button([this]() {
                    SetDataViewText(this->weatherData, "wind_speed_max", "wind speed max", " mph", "average", this->currentScene);
                }, Color::WHITE, Color::BLUE, "Wind Speed\nMax", Color::BLACK, Color::WHITE, 115, 130, 0, 90, 56, 0.6f, 0.6f)),
                std::shared_ptr<Button>(new Button([this]() {
                    SetDataViewText(this->weatherData, "precipitation_probability_max", "precipitation probability max", "%", "average", this->currentScene);
                }, Color::WHITE, Color::BLUE, "Precip Prob\nMax", Color::BLACK, Color::WHITE, 220, 130, 0, 90, 56, 0.6f, 0.6f)),
                std::shared_ptr<Button>(new Button([this]() { 
                    // Reset fields
                    auto text = (Text*)this->currentScene.topScreenObjects[0].get();
                    text->SetText(" ");
                    text->color = Color::BLACK;

                    ((TextInput*)this->currentScene.bottomScreenObjects[0].get())->Reset();
                    ((TextInput*)this->currentScene.bottomScreenObjects[1].get())->Reset();

                    // Go back
                    this->currentScene = this->viewWeatherDataPage;
                }, Color::WHITE, Color::BLUE, "Back ", Color::BLACK, Color::WHITE, 10, 200, 0, 64, 32, 0.5f, 0.5f)), 
            },

            // Update func
            this->baseScenePage1.Update
        };

        Scene viewWeatherDataSinglePointPage =
        { 
            // Name
            "View Weather Data Single Point Page",

            // Background color
            Color::GREY, Color::SILVER,

            // Top screen
            {
                std::shared_ptr<Text>(new Text(" ", 200, 20, 1.0f, 360, 1.1f, 1.1f, Color::BLACK, C2D_WithColor | C2D_AlignCenter | C2D_WordWrap)),
            },

            // Bottom screen
            { 
                std::shared_ptr<TextInput>(new TextInput(Color::WHITE, "Date... ", Color::BLACK, MAX_INPUT_SIZE, SWKBD_TYPE_NORMAL, 105, 20, 0, 125, 32, 0.65f, 0.65f)),
                std::shared_ptr<Button>(new Button([this]() { this->currentScene = this->viewWeatherDataPage; }, Color::WHITE, Color::BLUE, "Back ", Color::BLACK, Color::WHITE, 10, 200, 0, 64, 32, 0.5f, 0.5f)),
                std::shared_ptr<Button>(new Button([this]() { 
                    SetDataViewText(this->weatherData, "weather_code", "weather code", "", this->currentScene);
                }, Color::WHITE, Color::BLUE, "Weather\nCode", Color::BLACK, Color::WHITE, 10, 60, 0, 90, 56, 0.6f, 0.6f)),
                std::shared_ptr<Button>(new Button([this]() {
                    SetDataViewText(this->weatherData, "temperature_max", "temperature max", " °F", this->currentScene);
                }, Color::WHITE, Color::BLUE, "Temp Max", Color::BLACK, Color::WHITE, 115, 60, 0, 90, 56, 0.6f, 0.6f)),
                std::shared_ptr<Button>(new Button([this]() {
                    SetDataViewText(this->weatherData, "temperature_min", "temperature min", " °F", this->currentScene);
                }, Color::WHITE, Color::BLUE, "Temp Min", Color::BLACK, Color::WHITE, 220, 60, 0, 90, 56, 0.6f, 0.6f)),
                std::shared_ptr<Button>(new Button([this]() {
                    SetDataViewText(this->weatherData, "precipitation_sum", "precipitation sum", "\"", this->currentScene);
                }, Color::WHITE, Color::BLUE, "Precip Sum", Color::BLACK, Color::WHITE, 10, 130, 0, 90, 56, 0.6f, 0.6f)),
                std::shared_ptr<Button>(new Button([this]() {
                    SetDataViewText(this->weatherData, "wind_speed_max", "wind speed max", " mph", this->currentScene);
                }, Color::WHITE, Color::BLUE, "Wind Speed\nMax", Color::BLACK, Color::WHITE, 115, 130, 0, 90, 56, 0.6f, 0.6f)),
                std::shared_ptr<Button>(new Button([this]() {
                    SetDataViewText(this->weatherData, "precipitation_probability_max", "precipitation probability max", "%", this->currentScene);
                }, Color::WHITE, Color::BLUE, "Precip Prob\nMax", Color::BLACK, Color::WHITE, 220, 130, 0, 90, 56, 0.6f, 0.6f)),
                std::shared_ptr<Button>(new Button([this]() { 
                    // Reset fields
                    auto text = (Text*)this->currentScene.topScreenObjects[0].get();
                    text->SetText(" ");
                    text->color = Color::BLACK;

                    ((TextInput*)this->currentScene.bottomScreenObjects[0].get())->Reset();

                    // Go back
                    this->currentScene = this->viewWeatherDataPage;
                }, Color::WHITE, Color::BLUE, "Back ", Color::BLACK, Color::WHITE, 10, 200, 0, 64, 32, 0.5f, 0.5f)), 
            },

            // Update func
            this->baseScenePage1.Update
        };

        // Compare API pages
        Scene compareAPIDataPage =
        { 
            // Name
            "Compare API Data Page",

            // Background color
            Color::GREY, Color::SILVER,

            // Top screen
            this->baseScenePage1.topScreenObjects,

            // Bottom screen
            { 
                std::shared_ptr<Button>(new Button([this]() { this->currentScene = this->compareAPIDataMinPage; }, Color::WHITE, Color::BLUE, "Minimum ", Color::BLACK, Color::WHITE, 25, 60, 0, 125, 42, 0.65f, 0.65f)),
                std::shared_ptr<Button>(new Button([this]() { this->currentScene = this->compareAPIDataMaxPage; }, Color::WHITE, Color::BLUE, "Maximum ", Color::BLACK, Color::WHITE, 170, 60, 0, 125, 42, 0.65f, 0.65f)),
                std::shared_ptr<Button>(new Button([this]() { this->currentScene = this->compareAPIDataAveragePage; }, Color::WHITE, Color::BLUE, "Average ", Color::BLACK, Color::WHITE, 25, 112, 0, 125, 42, 0.65f, 0.65f)),
                std::shared_ptr<Button>(new Button([this]() { this->currentScene = this->compareAPIDataSinglePointPage; }, Color::WHITE, Color::BLUE, "Single Point", Color::BLACK, Color::WHITE, 170, 112, 0, 125, 42, 0.65f, 0.65f)),
                std::shared_ptr<Button>(new Button([this]() { this->currentScene = this->baseScenePage1; }, Color::WHITE, Color::BLUE, "Back ", Color::BLACK, Color::WHITE, 10, 200, 0, 64, 32, 0.5f, 0.5f)), 
            },

            // Update func
            this->baseScenePage1.Update
        };

        Scene compareAPIDataMinPage =
        { 
            // Name
            "Compare API Data Min Page",

            // Background color
            Color::GREY, Color::SILVER,

            // Top screen
            {
                std::shared_ptr<Text>(new Text(" ", 200, 20, 1.0f, 360, 1.0f, 1.0f, Color::BLACK, C2D_WithColor | C2D_AlignCenter | C2D_WordWrap)),
            },

            // Bottom screen
            { 
                std::shared_ptr<TextInput>(new TextInput(Color::WHITE, "Start date...", Color::BLACK, MAX_INPUT_SIZE, SWKBD_TYPE_NORMAL, 20, 20, 0, 125, 32, 0.65f, 0.65f)),
                std::shared_ptr<TextInput>(new TextInput(Color::WHITE, "End date...", Color::BLACK, MAX_INPUT_SIZE, SWKBD_TYPE_NORMAL, 175, 20, 0, 125, 32, 0.65f, 0.65f)),
                std::shared_ptr<TextInput>(new TextInput(Color::WHITE, "Latitude... ", Color::BLACK, MAX_INPUT_SIZE, SWKBD_TYPE_NORMAL, 20, 60, 0, 125, 32, 0.65f, 0.65f)),
                std::shared_ptr<TextInput>(new TextInput(Color::WHITE, "Longitude... ", Color::BLACK, MAX_INPUT_SIZE, SWKBD_TYPE_NORMAL, 175, 60, 0, 125, 32, 0.65f, 0.65f)),
                std::shared_ptr<Button>(new Button([this]() { 
                    SetDataViewTextWithAPI(this->weatherData, "weather_code", "weather code", "", "minimum", this->currentScene);
                }, Color::WHITE, Color::BLUE, "Weather\nCode", Color::BLACK, Color::WHITE, 10, 100, 0, 90, 56, 0.6f, 0.6f)),
                std::shared_ptr<Button>(new Button([this]() {
                    SetDataViewTextWithAPI(this->weatherData, "temperature_max", "temperature max", " °F", "minimum", this->currentScene);
                }, Color::WHITE, Color::BLUE, "Temp Max", Color::BLACK, Color::WHITE, 115, 100, 0, 90, 56, 0.6f, 0.6f)),
                std::shared_ptr<Button>(new Button([this]() {
                    SetDataViewTextWithAPI(this->weatherData, "temperature_min", "temperature min", " °F", "minimum", this->currentScene);
                }, Color::WHITE, Color::BLUE, "Temp Min", Color::BLACK, Color::WHITE, 220, 100, 0, 90, 56, 0.6f, 0.6f)),
                std::shared_ptr<Button>(new Button([this]() {
                    SetDataViewTextWithAPI(this->weatherData, "precipitation_sum", "precipitation sum", "\"", "minimum", this->currentScene);
                }, Color::WHITE, Color::BLUE, "Precip Sum", Color::BLACK, Color::WHITE, 115, 170, 0, 90, 56, 0.6f, 0.6f)),
                std::shared_ptr<Button>(new Button([this]() {
                    SetDataViewTextWithAPI(this->weatherData, "wind_speed_max", "wind speed max", " mph", "minimum", this->currentScene);
                }, Color::WHITE, Color::BLUE, "Wind Speed\nMax", Color::BLACK, Color::WHITE, 220, 170, 0, 90, 56, 0.6f, 0.6f)),
                std::shared_ptr<Button>(new Button([this]() { 
                    // Reset fields
                    auto text = (Text*)this->currentScene.topScreenObjects[0].get();
                    text->SetText(" ");
                    text->color = Color::BLACK;

                    ((TextInput*)this->currentScene.bottomScreenObjects[0].get())->Reset();
                    ((TextInput*)this->currentScene.bottomScreenObjects[1].get())->Reset();
                    ((TextInput*)this->currentScene.bottomScreenObjects[2].get())->Reset();
                    ((TextInput*)this->currentScene.bottomScreenObjects[3].get())->Reset();

                    // Go back
                    this->currentScene = this->compareAPIDataPage;
                }, Color::WHITE, Color::BLUE, "Back ", Color::BLACK, Color::WHITE, 10, 200, 0, 64, 32, 0.5f, 0.5f)), 
            },

            // Update func
            this->baseScenePage1.Update
        };

        Scene compareAPIDataMaxPage =
        { 
            // Name
            "Compare API Data Max Page",

            // Background color
            Color::GREY, Color::SILVER,

            // Top screen
            {
                std::shared_ptr<Text>(new Text(" ", 200, 20, 1.0f, 360, 1.0f, 1.0f, Color::BLACK, C2D_WithColor | C2D_AlignCenter | C2D_WordWrap)),
            },

            // Bottom screen
            { 
                std::shared_ptr<TextInput>(new TextInput(Color::WHITE, "Start date...", Color::BLACK, MAX_INPUT_SIZE, SWKBD_TYPE_NORMAL, 20, 20, 0, 125, 32, 0.65f, 0.65f)),
                std::shared_ptr<TextInput>(new TextInput(Color::WHITE, "End date...", Color::BLACK, MAX_INPUT_SIZE, SWKBD_TYPE_NORMAL, 175, 20, 0, 125, 32, 0.65f, 0.65f)),
                std::shared_ptr<TextInput>(new TextInput(Color::WHITE, "Latitude... ", Color::BLACK, MAX_INPUT_SIZE, SWKBD_TYPE_NORMAL, 20, 60, 0, 125, 32, 0.65f, 0.65f)),
                std::shared_ptr<TextInput>(new TextInput(Color::WHITE, "Longitude... ", Color::BLACK, MAX_INPUT_SIZE, SWKBD_TYPE_NORMAL, 175, 60, 0, 125, 32, 0.65f, 0.65f)),
                std::shared_ptr<Button>(new Button([this]() { 
                    SetDataViewTextWithAPI(this->weatherData, "weather_code", "weather code", "", "maximum", this->currentScene);
                }, Color::WHITE, Color::BLUE, "Weather\nCode", Color::BLACK, Color::WHITE, 10, 100, 0, 90, 56, 0.6f, 0.6f)),
                std::shared_ptr<Button>(new Button([this]() {
                    SetDataViewTextWithAPI(this->weatherData, "temperature_max", "temperature max", " °F", "maximum", this->currentScene);
                }, Color::WHITE, Color::BLUE, "Temp Max", Color::BLACK, Color::WHITE, 115, 100, 0, 90, 56, 0.6f, 0.6f)),
                std::shared_ptr<Button>(new Button([this]() {
                    SetDataViewTextWithAPI(this->weatherData, "temperature_min", "temperature min", " °F", "maximum", this->currentScene);
                }, Color::WHITE, Color::BLUE, "Temp Min", Color::BLACK, Color::WHITE, 220, 100, 0, 90, 56, 0.6f, 0.6f)),
                std::shared_ptr<Button>(new Button([this]() {
                    SetDataViewTextWithAPI(this->weatherData, "precipitation_sum", "precipitation sum", "\"", "maximum", this->currentScene);
                }, Color::WHITE, Color::BLUE, "Precip Sum", Color::BLACK, Color::WHITE, 115, 170, 0, 90, 56, 0.6f, 0.6f)),
                std::shared_ptr<Button>(new Button([this]() {
                    SetDataViewTextWithAPI(this->weatherData, "wind_speed_max", "wind speed max", " mph", "maximum", this->currentScene);
                }, Color::WHITE, Color::BLUE, "Wind Speed\nMax", Color::BLACK, Color::WHITE, 220, 170, 0, 90, 56, 0.6f, 0.6f)),
                std::shared_ptr<Button>(new Button([this]() { 
                    // Reset fields
                    auto text = (Text*)this->currentScene.topScreenObjects[0].get();
                    text->SetText(" ");
                    text->color = Color::BLACK;

                    ((TextInput*)this->currentScene.bottomScreenObjects[0].get())->Reset();
                    ((TextInput*)this->currentScene.bottomScreenObjects[1].get())->Reset();
                    ((TextInput*)this->currentScene.bottomScreenObjects[2].get())->Reset();
                    ((TextInput*)this->currentScene.bottomScreenObjects[3].get())->Reset();

                    // Go back
                    this->currentScene = this->compareAPIDataPage;
                }, Color::WHITE, Color::BLUE, "Back ", Color::BLACK, Color::WHITE, 10, 200, 0, 64, 32, 0.5f, 0.5f)), 
            },

            // Update func
            this->baseScenePage1.Update
        };

        Scene compareAPIDataAveragePage =
        { 
            // Name
            "Compare API Data Average Page",

            // Background color
            Color::GREY, Color::SILVER,

            // Top screen
            {
                std::shared_ptr<Text>(new Text(" ", 200, 20, 1.0f, 360, 1.0f, 1.0f, Color::BLACK, C2D_WithColor | C2D_AlignCenter | C2D_WordWrap)),
            },

            // Bottom screen
            { 
                std::shared_ptr<TextInput>(new TextInput(Color::WHITE, "Start date...", Color::BLACK, MAX_INPUT_SIZE, SWKBD_TYPE_NORMAL, 20, 20, 0, 125, 32, 0.65f, 0.65f)),
                std::shared_ptr<TextInput>(new TextInput(Color::WHITE, "End date...", Color::BLACK, MAX_INPUT_SIZE, SWKBD_TYPE_NORMAL, 175, 20, 0, 125, 32, 0.65f, 0.65f)),
                std::shared_ptr<TextInput>(new TextInput(Color::WHITE, "Latitude... ", Color::BLACK, MAX_INPUT_SIZE, SWKBD_TYPE_NORMAL, 20, 60, 0, 125, 32, 0.65f, 0.65f)),
                std::shared_ptr<TextInput>(new TextInput(Color::WHITE, "Longitude... ", Color::BLACK, MAX_INPUT_SIZE, SWKBD_TYPE_NORMAL, 175, 60, 0, 125, 32, 0.65f, 0.65f)),
                std::shared_ptr<Button>(new Button([this]() { 
                    SetDataViewTextWithAPI(this->weatherData, "weather_code", "weather code", "", "average", this->currentScene);
                }, Color::WHITE, Color::BLUE, "Weather\nCode", Color::BLACK, Color::WHITE, 10, 100, 0, 90, 56, 0.6f, 0.6f)),
                std::shared_ptr<Button>(new Button([this]() {
                    SetDataViewTextWithAPI(this->weatherData, "temperature_max", "temperature max", " °F", "average", this->currentScene);
                }, Color::WHITE, Color::BLUE, "Temp Max", Color::BLACK, Color::WHITE, 115, 100, 0, 90, 56, 0.6f, 0.6f)),
                std::shared_ptr<Button>(new Button([this]() {
                    SetDataViewTextWithAPI(this->weatherData, "temperature_min", "temperature min", " °F", "average", this->currentScene);
                }, Color::WHITE, Color::BLUE, "Temp Min", Color::BLACK, Color::WHITE, 220, 100, 0, 90, 56, 0.6f, 0.6f)),
                std::shared_ptr<Button>(new Button([this]() {
                    SetDataViewTextWithAPI(this->weatherData, "precipitation_sum", "precipitation sum", "\"", "average", this->currentScene);
                }, Color::WHITE, Color::BLUE, "Precip Sum", Color::BLACK, Color::WHITE, 115, 170, 0, 90, 56, 0.6f, 0.6f)),
                std::shared_ptr<Button>(new Button([this]() {
                    SetDataViewTextWithAPI(this->weatherData, "wind_speed_max", "wind speed max", " mph", "average", this->currentScene);
                }, Color::WHITE, Color::BLUE, "Wind Speed\nMax", Color::BLACK, Color::WHITE, 220, 170, 0, 90, 56, 0.6f, 0.6f)),
                std::shared_ptr<Button>(new Button([this]() { 
                    // Reset fields
                    auto text = (Text*)this->currentScene.topScreenObjects[0].get();
                    text->SetText(" ");
                    text->color = Color::BLACK;

                    ((TextInput*)this->currentScene.bottomScreenObjects[0].get())->Reset();
                    ((TextInput*)this->currentScene.bottomScreenObjects[1].get())->Reset();
                    ((TextInput*)this->currentScene.bottomScreenObjects[2].get())->Reset();
                    ((TextInput*)this->currentScene.bottomScreenObjects[3].get())->Reset();

                    // Go back
                    this->currentScene = this->compareAPIDataPage;
                }, Color::WHITE, Color::BLUE, "Back ", Color::BLACK, Color::WHITE, 10, 200, 0, 64, 32, 0.5f, 0.5f)), 
            },

            // Update func
            this->baseScenePage1.Update
        };

        Scene compareAPIDataSinglePointPage =
        { 
            // Name
            "Compare API Data Single Point Page",

            // Background color
            Color::GREY, Color::SILVER,

            // Top screen
            {
                std::shared_ptr<Text>(new Text(" ", 200, 20, 1.0f, 360, 1.0f, 1.0f, Color::BLACK, C2D_WithColor | C2D_AlignCenter | C2D_WordWrap)),
            },

            // Bottom screen
            { 
                std::shared_ptr<TextInput>(new TextInput(Color::WHITE, "Date... ", Color::BLACK, MAX_INPUT_SIZE, SWKBD_TYPE_NORMAL, 97.5, 20, 0, 125, 32, 0.65f, 0.65f)),
                std::shared_ptr<TextInput>(new TextInput(Color::WHITE, "Latitude... ", Color::BLACK, MAX_INPUT_SIZE, SWKBD_TYPE_NORMAL, 20, 60, 0, 125, 32, 0.65f, 0.65f)),
                std::shared_ptr<TextInput>(new TextInput(Color::WHITE, "Longitude... ", Color::BLACK, MAX_INPUT_SIZE, SWKBD_TYPE_NORMAL, 175, 60, 0, 125, 32, 0.65f, 0.65f)),
                std::shared_ptr<Button>(new Button([this]() { 
                    SetDataViewTextWithAPI(this->weatherData, "weather_code", "weather code", "", this->currentScene);
                }, Color::WHITE, Color::BLUE, "Weather\nCode", Color::BLACK, Color::WHITE, 10, 100, 0, 90, 56, 0.6f, 0.6f)),
                std::shared_ptr<Button>(new Button([this]() {
                    SetDataViewTextWithAPI(this->weatherData, "temperature_max", "temperature max", " °F", this->currentScene);
                }, Color::WHITE, Color::BLUE, "Temp Max", Color::BLACK, Color::WHITE, 115, 100, 0, 90, 56, 0.6f, 0.6f)),
                std::shared_ptr<Button>(new Button([this]() {
                    SetDataViewTextWithAPI(this->weatherData, "temperature_min", "temperature min", " °F", this->currentScene);
                }, Color::WHITE, Color::BLUE, "Temp Min", Color::BLACK, Color::WHITE, 220, 100, 0, 90, 56, 0.6f, 0.6f)),
                std::shared_ptr<Button>(new Button([this]() {
                    SetDataViewTextWithAPI(this->weatherData, "precipitation_sum", "precipitation sum", "\"", this->currentScene);
                }, Color::WHITE, Color::BLUE, "Precip Sum", Color::BLACK, Color::WHITE, 115, 170, 0, 90, 56, 0.6f, 0.6f)),
                std::shared_ptr<Button>(new Button([this]() {
                    SetDataViewTextWithAPI(this->weatherData, "wind_speed_max", "wind speed max", " mph", this->currentScene);
                }, Color::WHITE, Color::BLUE, "Wind Speed\nMax", Color::BLACK, Color::WHITE, 220, 170, 0, 90, 56, 0.6f, 0.6f)),
                std::shared_ptr<Button>(new Button([this]() { 
                    // Reset fields
                    auto text = (Text*)this->currentScene.topScreenObjects[0].get();
                    text->SetText(" ");
                    text->color = Color::BLACK;

                    ((TextInput*)this->currentScene.bottomScreenObjects[0].get())->Reset();
                    ((TextInput*)this->currentScene.bottomScreenObjects[1].get())->Reset();
                    ((TextInput*)this->currentScene.bottomScreenObjects[2].get())->Reset();

                    // Go back
                    this->currentScene = this->compareAPIDataPage;
                }, Color::WHITE, Color::BLUE, "Back ", Color::BLACK, Color::WHITE, 10, 200, 0, 64, 32, 0.5f, 0.5f)), 
            },

            // Update func
            this->baseScenePage1.Update
        };

        Scene createHistogramPage =
        { 
            // Name
            "Create Histogram Page",

            // Background color
            Color::GREY, Color::SILVER,

            // Top screen
            {},

            // Bottom screen
            { 
                std::shared_ptr<TextInput>(new TextInput(Color::WHITE, "Start date...", Color::BLACK, MAX_INPUT_SIZE, SWKBD_TYPE_NORMAL, 20, 20, 0, 125, 32, 0.65f, 0.65f)),
                std::shared_ptr<TextInput>(new TextInput(Color::WHITE, "End date...", Color::BLACK, MAX_INPUT_SIZE, SWKBD_TYPE_NORMAL, 175, 20, 0, 125, 32, 0.65f, 0.65f)),
                std::shared_ptr<Button>(new Button([this]() { 
                    CreateHistogram(this->weatherData, "weather_code", "weather code", "", this->currentScene);
                }, Color::WHITE, Color::BLUE, "Weather\nCode", Color::BLACK, Color::WHITE, 10, 60, 0, 90, 56, 0.6f, 0.6f)),
                std::shared_ptr<Button>(new Button([this]() {
                    CreateHistogram(this->weatherData, "temperature_max", "temperature max", "°F", this->currentScene);
                }, Color::WHITE, Color::BLUE, "Temp Max", Color::BLACK, Color::WHITE, 115, 60, 0, 90, 56, 0.6f, 0.6f)),
                std::shared_ptr<Button>(new Button([this]() {
                    CreateHistogram(this->weatherData, "temperature_min", "temperature min", "°F", this->currentScene);
                }, Color::WHITE, Color::BLUE, "Temp Min", Color::BLACK, Color::WHITE, 220, 60, 0, 90, 56, 0.6f, 0.6f)),
                std::shared_ptr<Button>(new Button([this]() {
                    CreateHistogram(this->weatherData, "precipitation_sum", "precipitation sum", "in", this->currentScene);
                }, Color::WHITE, Color::BLUE, "Precip Sum", Color::BLACK, Color::WHITE, 10, 130, 0, 90, 56, 0.6f, 0.6f)),
                std::shared_ptr<Button>(new Button([this]() {
                    CreateHistogram(this->weatherData, "wind_speed_max", "wind speed max", "mph", this->currentScene);
                }, Color::WHITE, Color::BLUE, "Wind Speed\nMax", Color::BLACK, Color::WHITE, 115, 130, 0, 90, 56, 0.6f, 0.6f)),
                std::shared_ptr<Button>(new Button([this]() {
                    CreateHistogram(this->weatherData, "precipitation_probability_max", "precipitation probability max", "%", this->currentScene);
                }, Color::WHITE, Color::BLUE, "Precip Prob\nMax", Color::BLACK, Color::WHITE, 220, 130, 0, 90, 56, 0.6f, 0.6f)),
                std::shared_ptr<Button>(new Button([this]() { 
                    // Reset fields
                    this->currentScene.topScreenObjects.clear();
                    ((TextInput*)this->currentScene.bottomScreenObjects[0].get())->Reset();
                    ((TextInput*)this->currentScene.bottomScreenObjects[1].get())->Reset();

                    // Go back
                    this->currentScene = this->baseScenePage2;
                }, Color::WHITE, Color::BLUE, "Back ", Color::BLACK, Color::WHITE, 10, 200, 0, 64, 32, 0.5f, 0.5f)), 
            },

            // Update func
            this->baseScenePage1.Update
        };
};