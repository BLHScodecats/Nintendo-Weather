#include "ui/textinput.hpp"

TextInput::TextInput() {}

TextInput::TextInput(u32 backColor, std::string hintText, u32 textColor, unsigned int maxInputLength, SwkbdType inputType,
                     float x, float y, float z, float width, float height, float scaleX, float scaleY)
{
    this->backColor = backColor;
    this->hintText = hintText;
    this->inputType = inputType;
    this->x = x;
    this->y = y;
    this->z = z;
    this->width = width;
    this->height = height;

    this->text = std::shared_ptr<Text>(new Text(hintText, this->x + (this->width * 0.5f), this->y + (this->height * 0.5f), this->z - 0.3f,
                                                this->width, scaleX, scaleY, textColor, C2D_WithColor));
}

TextInput::~TextInput()
{

}

std::string TextInput::GetCurrentText()
{
    return this->currentText;
}

void TextInput::Reset()
{
    this->text->SetText(this->hintText);
}

void TextInput::Update(Input &input)
{
    this->text->x = this->x;
    this->text->y = this->y + (this->height * 0.5f) - (this->text->GetHeight() * 0.5f);
    this->text->z = this->z - 0.3f;
    
    this->text->width = this->width;

    // Figure out if text box was pressed
    if (input.buttonsDown & KEY_TOUCH)
    {
        // Detect if touch was in bounds
        if (input.touchPos.px > this->x && input.touchPos.px < this->x + this->width &&
            input.touchPos.py > this->y && input.touchPos.py < this->y + this->height)
        {
            // Open keyboard and get input
            char input[MAX_INPUT_SIZE];
            this->keyboard.getInput(input, MAX_INPUT_SIZE, this->inputType, this->hintText);

            if (this->keyboard.lastPressedButton == SWKBD_BUTTON_CONFIRM)
            {
                this->currentText = std::string(input);
                this->text->SetText(currentText + " ");
            }
        }
    }
}

void TextInput::Render(float depthMult)
{
    float xPos = this->x;
    if (this->z != 0.0f)
        xPos += depthMult * depthOffset * this->z;
        
    C2D_DrawRectSolid(xPos, this->y, 0.0f, this->width, this->height, this->backColor);

    this->text->Render(depthMult);
}