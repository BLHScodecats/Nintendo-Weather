#include "date.hpp"
#include <iostream>

Date::Date()
{
    this->year = 0;
    this->month = 0;
    this->day = 0;
}

Date::Date(int year, int month, int day)
{
    this->year = year;
    this->month = month;
    this->day = day;
}

Date::~Date()
{

}

bool Date::operator<(const Date &other) const
{
    return this->year < other.year ||
           (this->year == other.year && this->month < other.month) ||
           (this->year == other.year && this->month == other.month && this->day < other.day);
}

bool Date::operator <=(const Date &other) const
{
    return this->year < other.year ||
           (this->year == other.year && this->month < other.month) ||
           (this->year == other.year && this->month == other.month && this->day <= other.day);
}

bool Date::operator>(const Date &other) const
{
    return this->year > other.year ||
          (this->year == other.year && this->month > other.month) ||
          (this->year == other.year && this->month == other.month && this->day > other.day);
}

bool Date::operator >=(const Date &other) const
{
    return this->year > other.year ||
          (this->year == other.year && this->month > other.month) ||
          (this->year == other.year && this->month == other.month && this->day >= other.day);
}

bool Date::operator==(const Date &other) const
{
    return this->year == other.year && this->month == other.month && this->day == other.day;
}

std::string Date::toString() const
{
    std::stringstream s;
    s << std::setfill('0') << std::setw(4) << year << std::setw(0) <<  "-" << std::setw(2) << month << std::setw(0) << "-" << std::setw(2) << day << std::setw(0);
    return s.str();
}