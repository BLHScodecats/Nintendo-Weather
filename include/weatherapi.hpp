#pragma once

#include "global.hpp"
#include "datapoint.hpp"
#include "json.hpp"

namespace WeatherDataAPIHandler
{
    WeatherDataPoint GetData(double latitude, double longitude, Date date);
    std::vector<WeatherDataPoint> GetData(double latitude, double longitude, Date startDate, Date endDate);
    size_t HTTPResultWrite(void* ptr, size_t size, size_t nmemb, std::string* s);
    std::string GetJSONFromAPI(double latitude, double longitude, Date date);
    std::string GetJSONFromAPI(double latitude, double longitude, Date startDate, Date endDate);
}