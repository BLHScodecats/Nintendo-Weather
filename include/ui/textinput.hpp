#pragma once

#include "global.hpp"
#include "object.hpp"
#include "text.hpp"
#include "keyboard.hpp"

class TextInput : public Object
{
    public:
        TextInput();
        TextInput(u32 backColor, std::string hintText, u32 textColor, unsigned int maxInputLength, SwkbdType inputType,
                  float x, float y, float z, float width, float height, float scaleX, float scaleY);
        ~TextInput();

        u32 backColor;
        float width, height;

        std::string GetCurrentText();
        void Reset();

        void Update(Input &input) override;
        void Render(float depthMult) override;
    private:
        std::shared_ptr<Text> text;
        Keyboard keyboard;

        SwkbdType inputType;
        std::string hintText;
        std::string currentText;
};