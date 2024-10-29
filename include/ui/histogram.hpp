#pragma once

#include "global.hpp"
#include "object.hpp"
#include "datapoint.hpp"

class Histogram : public Object
{
    public:
        Histogram();
        Histogram(std::vector<WeatherDataPoint> &data, std::string dataPoint, int horizontalIncrements, int verticalIncrements,
                  std::string horizontalLabel, std::string verticalLabel, u32 lineColor, u32 barColor, u32 textColor, 
                  float x, float y, float z, float width, float height);
        ~Histogram();

        u32 lineColor, barColor1, barColor2, textColor;
        float width, height;
        int horizontalIncrements, verticalIncrements;

        void Update(Input &input) override;
        void Render(float depthMult) override;
    private:
        std::vector<std::shared_ptr<Object> > children;
};