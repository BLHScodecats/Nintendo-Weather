#include "ui/histogram.hpp"

Histogram::Histogram() {}

Histogram::Histogram(std::vector<WeatherDataPoint> &data, std::string dataPoint, int horizontalIncrements, int verticalIncrements,
                     std::string horizontalLabel, std::string verticalLabel, u32 lineColor, u32 barColor, u32 textColor, 
                     float x, float y, float z, float width, float height)
{
    this->lineColor = lineColor;
    this->barColor1 = barColor;
    this->barColor2 = barColor - Color::COLOR_ADJUST;
    this->textColor = textColor;
    this->width = width;
    this->height = height;
    this->horizontalIncrements = horizontalIncrements;
    this->verticalIncrements = verticalIncrements;

    this->x = x;
    this->y = y;
    this->z = z;

    // now actually create it lol
}

Histogram::~Histogram() {}

void Histogram::Update(Input &input) 
{
    // Update children
    for (unsigned int i = 0; i < this->children.size(); i++)
    {
        this->children[i]->Update(input);
    }
}

void Histogram::Render(float depthMult)
{
    // Render lines
    for (int i = 0; i < this->horizontalIncrements + 1; i++)
    {
        int xPos = this->x + (i * (this->width / this->horizontalIncrements));
        if (this->z != 0.0f)
            xPos += depthMult * depthOffset * this->z;

        C2D_DrawLine(xPos, this->y, this->lineColor, xPos, (this->y + this->height), this->lineColor, 1.5f, 0);
    }
    for (int i = 0; i < this->verticalIncrements + 1; i++)
    {
        int yPos = this->y + (i * (this->height / this->verticalIncrements));

        float xPos = this->x;
        if (this->z != 0.0f)
            xPos += depthMult * depthOffset * this->z;

        C2D_DrawLine(xPos, yPos, this->lineColor, xPos + this->width, yPos, this->lineColor, 1.5f, 0);
    }

    // Render children
    for (unsigned int i = 0; i < this->children.size(); i++)
    {
        this->children[i]->Render(depthMult);
    }
}