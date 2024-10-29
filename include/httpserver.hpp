#pragma once

#include "global.hpp"
#include "date.hpp"
#include "datapoint.hpp"

#define PORT 8080

class HTTPServer
{
    public:
        HTTPServer();
        HTTPServer(std::map<Date, WeatherDataPoint>* weatherData, bool* weatherDataLock, s32 mainThreadPriority);
        ~HTTPServer();

        bool runThread = true;

        void StartServer();
        void EndServer();
        void RunServer();
    private:
        std::map<Date, WeatherDataPoint>* weatherData;
        bool* weatherDataLock;

        int mainThreadPriority;
        Thread serverThread;
};

// Thread creation requires free function
inline void RunServerHelper(void* arg)
{
    ((HTTPServer*)arg)->RunServer();
}