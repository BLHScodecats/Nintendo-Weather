#pragma once

#include "global.hpp"
#include "object.hpp"
#include "text.hpp"

enum ButtonState
{
    Up, Down
};

class Button : public Object
{
    public:
        Button();
        Button(std::function<void()> OnPress, u32 backColorBase, u32 backColorPress, std::string text,
               u32 textColorBase, u32 textColorPress, float x, float y, float z, float width, float height, float scaleX, float scaleY);
        ~Button();

        std::function<void()> OnPress;
        u32 backColorBase, backColorPress, textColorBase, textColorPress;
        float width, height;

        void Update(Input &input) override;
        void Render(float depthMult) override;
    private:
        std::shared_ptr<Text> text;
        ButtonState buttonState = ButtonState::Up;
};