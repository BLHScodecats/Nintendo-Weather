#pragma once

#include "global.hpp"

class Input
{
    public:
        Input()
        {
            this->buttonsDown = this->buttonsHeld = this->buttonsUp = 0;
            this->circlePos = this->cStickPos = {};
            this->touchPos = {};
        }

        void GatherInputs()
        {
            hidScanInput();

            this->buttonsDown = hidKeysDown();
            this->buttonsHeld = hidKeysHeld();
            this->buttonsUp = hidKeysUp();

            hidCircleRead(&this->circlePos);
            hidCstickRead(&this->cStickPos);
            hidTouchRead(&this->touchPos);
        }

        u32 buttonsDown, buttonsHeld, buttonsUp;
        circlePosition circlePos, cStickPos;
        touchPosition touchPos;
};