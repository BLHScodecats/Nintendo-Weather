#pragma once

#include "global.hpp"

class Keyboard
{
    public:
        Keyboard();
        ~Keyboard();

        void openKeyboard(char* inputBuffer, int maxChars, SwkbdType keyboardType, SwkbdValidInput inputValidation, bool checkInputValid, std::string leftButtonText, std::string rightButtonText, std::string hintText);
        void getInput(char* out, int maxChars, SwkbdType keyboardType, std::string hintText, bool checkValidInput = false);
        static SwkbdCallbackResult CheckInputValid(void* user, const char** ppMessage, const char* text, size_t textlen);

        SwkbdButton lastPressedButton;
    private:
        static const u32 swkbdFeatures = 0;

        SwkbdState swkbd;
        SwkbdStatusData swkbdStatus;
};