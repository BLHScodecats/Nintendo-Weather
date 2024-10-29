#pragma once

#include "global.hpp"
#include "object.hpp"

struct Scene
{
    std::string name;
    u32 topBackgroundColor, bottomBackgroundColor;
    std::vector<std::shared_ptr<Object> > topScreenObjects, bottomScreenObjects;
    std::function<void()> Update;

    bool operator==(const Scene &other)
    {
        return this->name == other.name;
    }
};