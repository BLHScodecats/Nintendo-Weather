#pragma once

#include "global.hpp"
#include "ui/object.hpp"

class Text : public Object
{
    public:
        Text();
        Text(std::string text, float x, float y, float z, float width, 
             float scaleX = 1.0f, float scaleY = 1.0f, u32 color = Color::BLACK, int drawArgs = C2D_WithColor);
        ~Text();

        void SetText(std::string text);
        float GetHeight();

        float width;
        float scaleX, scaleY;
        u32 color;
        int drawArgs;

        void Update(Input &input) override;
        void Render(float depthMult) override;
    private:
        C2D_Text textObj;
        C2D_TextBuf textBuf;
};