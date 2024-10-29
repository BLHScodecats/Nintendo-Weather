#pragma once

#include "global.hpp"
#include "ui/input.hpp"

class Object
{
    public:
        virtual void Update(Input &input) {}
        virtual void Render(float depthMult) {}

        float x, y, z;

        constexpr static float depthOffset = 3;
};