#pragma once

#include "global.hpp"

class Date
{
    public:
        Date();
        Date(int year, int month, int day);
        ~Date();

        int year;
        int month;
        int day;

        bool operator <(const Date &other) const;
        bool operator <=(const Date &other) const;
        bool operator >(const Date &other) const;
        bool operator >=(const Date &other) const;
        bool operator ==(const Date &other) const;
        std::string toString() const;
};