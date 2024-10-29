#include "ui/button.hpp"

Button::Button()
{
    this->OnPress = std::function<void()>{};
    this->backColorBase = Color::SILVER;
    this->backColorPress = Color::GREY;
    this->textColorBase = Color::WHITE;
    this->textColorPress = Color::WHITE;
    this->x = this->y = this->z = 0;
    this->width = 75;
    this->height = 35;

    this->text = std::shared_ptr<Text>(new Text());
}

Button::Button(std::function<void()> OnPress, u32 backColorBase, u32 backColorPress, std::string text,
               u32 textColorBase, u32 textColorPress, float x, float y, float z, float width, float height, float scaleX, float scaleY)
{
    this->OnPress = OnPress;
    this->backColorBase = backColorBase;
    this->backColorPress = backColorPress;
    this->textColorBase = textColorBase;
    this->textColorPress = textColorPress;
    this->x = x;
    this->y = y;
    this->z = z;
    this->width = width;
    this->height = height;

    this->text = std::shared_ptr<Text>(new Text(text, this->x + (this->width * 0.5f), this->y + (this->height * 0.25f), this->z - 0.3f,
                                                this->width, scaleX, scaleY, this->textColorBase, C2D_WithColor | C2D_AlignCenter | C2D_WordWrap));
}

Button::~Button() {}

void Button::Update(Input &input)
{
    // Set text to center of button
    this->text->x = this->x + (this->width * 0.5f);
    this->text->y = this->y + (this->height * 0.5f) - this->text->GetHeight() * 0.5f;
    this->text->z = this->z - 0.3f;
    
    this->text->width = this->width;

    // Switch button state based on touch input
    if (input.buttonsDown & KEY_TOUCH)
    {
        // Detect if touch was in bounds
        if (input.touchPos.px > this->x && input.touchPos.px < this->x + this->width &&
            input.touchPos.py > this->y && input.touchPos.py < this->y + this->height)
        {
            this->buttonState = ButtonState::Down;
        }
    }
    if (input.buttonsUp & KEY_TOUCH && this->buttonState == ButtonState::Down)
    {
        this->OnPress();
        this->buttonState = ButtonState::Up;
    }
    else if (!(input.touchPos.px > this->x && input.touchPos.px < this->x + this->width &&
          input.touchPos.py > this->y && input.touchPos.py < this->y + this->height))
    {
        this->buttonState = ButtonState::Up;
    }
}

void Button::Render(float depthMult)
{
    float xPos = this->x;
    if (this->z != 0.0f)
        xPos += depthMult * depthOffset * this->z;
        

    if (this->buttonState == ButtonState::Up)
    {
        this->text->color = this->textColorBase;
        C2D_DrawRectSolid(xPos, this->y, 0.0f, this->width, this->height, this->backColorBase);
    }
    else
    {
        this->text->color = this->textColorPress;
        C2D_DrawRectSolid(xPos, this->y, 0.0f, this->width, this->height, this->backColorPress);
    }

    this->text->Render(depthMult);
}