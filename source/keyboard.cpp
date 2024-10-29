#include "ui/keyboard.hpp"

Keyboard::Keyboard()
{
    this->lastPressedButton = SWKBD_BUTTON_NONE;
}

Keyboard::~Keyboard()
{

}

void Keyboard::openKeyboard(char* inputBuffer, int maxChars, SwkbdType keyboardType, SwkbdValidInput inputValidation, bool checkInputValid, std::string leftButtonText = "", std::string rightButtonText = "", std::string hintText = "")
{
    swkbdInit(&this->swkbd, keyboardType, 2, -1);
    swkbdSetButton(&this->swkbd, SWKBD_BUTTON_LEFT, leftButtonText.c_str(), false);
    swkbdSetButton(&this->swkbd, SWKBD_BUTTON_RIGHT, rightButtonText.c_str(), true);
    swkbdSetHintText(&this->swkbd, hintText.c_str());
    swkbdSetFeatures(&this->swkbd, swkbdFeatures);
    swkbdSetValidation(&this->swkbd, inputValidation, 0, 0);
    if (checkInputValid)
        swkbdSetFilterCallback(&this->swkbd, CheckInputValid, NULL);
    else
        swkbdSetFilterCallback(&this->swkbd, NULL, NULL);

    static bool reload = false;
    swkbdSetStatusData(&this->swkbd, &this->swkbdStatus, reload, true);
    reload = true;

    this->lastPressedButton = swkbdInputText(&this->swkbd, inputBuffer, sizeof(char) * maxChars);
}

void Keyboard::getInput(char* out, int maxChars, SwkbdType keyboardType, std::string hintText, bool checkInputValid)
{
    openKeyboard(out, maxChars, keyboardType, (checkInputValid ? SWKBD_ANYTHING : SWKBD_NOTEMPTY_NOTBLANK), checkInputValid, "Cancel", "Enter", hintText);

    // Output the inputted text to mimic typing in terminal
    std::cout << out << std::endl;
}

SwkbdCallbackResult Keyboard::CheckInputValid(void* user, const char** ppMessage, const char* text, size_t textlen)
{
    std::string t = std::string(text); // Cast to string for easier checking
	if (t.empty())
	{
		*ppMessage = "Please enter a value.";
		return SWKBD_CALLBACK_CONTINUE;
	}

	return SWKBD_CALLBACK_OK;
}