#include "ui/text.hpp"

Text::Text()
{
    this->textObj = {};
    this->textBuf = C2D_TextBufNew(1);
    this->x = this->y = this->z = 0;
    this->width = 100;
    this->scaleX = this->scaleY = 1.0f;
    this->color = Color::BLACK;
}

Text::Text(std::string text, float x, float y, float z, float width, float scaleX, float scaleY, u32 color, int drawArgs)
{
    this->textObj = {};

    this->textBuf = C2D_TextBufNew(text.size());
    C2D_TextParse(&this->textObj, this->textBuf, text.c_str());
    C2D_TextOptimize(&this->textObj);

    this->x = x;
    this->y = y;
    this->z = z;
    this->width = width;
    this->scaleX = scaleX;
    this->scaleY = scaleY;
    this->color = color;
    this->drawArgs = drawArgs;
}

Text::~Text() {}

void Text::SetText(std::string text)
{
    C2D_TextBufDelete(this->textBuf);
    this->textBuf = C2D_TextBufNew(strlen(text.c_str()));
    C2D_TextParse(&this->textObj, this->textBuf, text.c_str());
    C2D_TextOptimize(&this->textObj);
}

float Text::GetHeight()
{
    float width = 0.0f, height = 0.0f;
    C2D_TextGetDimensions(&this->textObj, this->scaleX, this->scaleY, &width, &height);
    return height;
}

void Text::Update(Input &input) {}

void Text::Render(float depthMult)
{
    float xPos = this->x;
    if (this->z != 0.0f)
        xPos += depthMult * depthOffset * this->z;

    C2D_DrawText(&this->textObj, this->drawArgs, xPos, this->y, 0.0f,
                 this->scaleX, this->scaleY, this->color, this->width);
}