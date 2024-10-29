#pragma once

#include "global.hpp"
#include "date.hpp"
#include "json.hpp"

struct WeatherDataPoint
{
    Date date;
    int weather_code;
    double temperature_max;
    double temperature_min;
    double precipitation_sum;
    double wind_speed_max;
    int precipitation_probability_max;

    WeatherDataPoint()
    {
        this->date = Date();
        this->weather_code = 0;
        this->temperature_max = 0;
        this->temperature_min = 0;
        this->precipitation_sum = 0;
        this->wind_speed_max = 0;
        this->precipitation_probability_max = 0;
    };

    WeatherDataPoint(Date date, int weather_code, float temperature_max, float temperature_min,
                     double precipitation_sum, double wind_speed_max, int precipitation_probability_max)
    {
        this->date = date;
        this->weather_code = weather_code;
        this->temperature_max = temperature_max;
        this->temperature_min = temperature_min;
        this->precipitation_sum = precipitation_sum;
        this->wind_speed_max = wind_speed_max;
        this->precipitation_probability_max = precipitation_probability_max;
    }

    static void readFromFile(std::string filename, std::map<Date, WeatherDataPoint>* weatherDataPoints)
    {
        std::ifstream file;
        file.open(DATA_DIR + filename);

        std::vector<WeatherDataPoint> dataPoints;
        while (true)
        {
            std::string input;
            getline(file, input);

            if (file.fail())
                break;

            // Read data into vector of data points, using item label to determine data type
            std::vector<std::string> splitData = splitString(input, ' ');
            if (dataPoints.empty()) dataPoints.resize(splitData.size() - 1);
            std::string label = splitData[0];

            if (label == "date:")
            {
                for (unsigned int i = 1; i < splitData.size(); i++)
                {
                    std::vector<std::string> date = splitString(splitData[i], '-');
                    dataPoints[i-1].date = Date(stoi(date[0]), stoi(date[1]), stoi(date[2]));
                }
            }
            else if (label == "weather_code:")
            {
                for (unsigned int i = 1; i < splitData.size(); i++)
                {
                    dataPoints[i-1].weather_code = stoi(splitData[i]);
                }
            }
            else if (label == "temperature_max:")
            {
                for (unsigned int i = 1; i < splitData.size(); i++)
                {
                    dataPoints[i-1].temperature_max = stod(splitData[i]);
                }
            }
            else if (label == "temperature_min:")
            {
                for (unsigned int i = 1; i < splitData.size(); i++)
                {
                    dataPoints[i-1].temperature_min = stod(splitData[i]);
                }
            }
            else if (label == "precipitation_sum:")
            {
                for (unsigned int i = 1; i < splitData.size(); i++)
                {
                    dataPoints[i-1].precipitation_sum = stod(splitData[i]);
                }
            }
            else if (label == "wind_speed_max:")
            {
                for (unsigned int i = 1; i < splitData.size(); i++)
                {
                    dataPoints[i-1].wind_speed_max = stod(splitData[i]);
                }
            }
            else if (label == "precipitation_probability_max:")
            {
                for (unsigned int i = 1; i < splitData.size(); i++)
                {
                    dataPoints[i-1].precipitation_probability_max = stoi(splitData[i]);
                }
            }
            else // Invalid file, return empty
            {
                dataPoints.clear();
                break;
            }
        }
        file.close();

        // Fill weatherDataPoints with newly read data
        for (auto d : dataPoints)
        {
            weatherDataPoints->insert(std::pair<Date, WeatherDataPoint>(d.date, d));
        }
    }

    std::string toString() const
    {
        std::stringstream s;
        s << "Date: " << this->date.toString() << std::endl;
        s << "Weather Code: " << this->weather_code << std::endl;
        s << "Temperature Max: " << this->temperature_max << std::endl;
        s << "Temperature Min: " << this->temperature_min << std::endl;
        s << "Precipitation Sum: " << this->precipitation_sum << std::endl;
        s << "Wind Speed Max: " << this->wind_speed_max << std::endl;
        s << "Precipitation Probability Max: " << this->precipitation_probability_max << std::endl;
        return s.str();
    }
};

inline void to_json(nlohmann::json &j, const WeatherDataPoint p)
{
    j = nlohmann::json
    { 
        { "date", p.date.toString() }, 
        { "weather_code", p.weather_code }, 
        { "temperature_max", p.temperature_max }, 
        { "temperature_min", p.temperature_min }, 
        { "precipitation_sum", p.precipitation_sum }, 
        { "wind_speed_max", p.wind_speed_max }, 
        { "precipitation_probability_max", p.precipitation_probability_max } 
    };
}

inline WeatherDataPoint getDataPointFromIndex(std::map<Date, WeatherDataPoint> &in, int index)
{
    auto itr = in.begin();
    for (int i = 0; i < index; i++)
    {
        itr++;
        if (itr == in.end())
            return WeatherDataPoint();
    }
    return itr->second;
}

inline std::vector<WeatherDataPoint> weatherDataMapToVector(std::map<Date, WeatherDataPoint> &in)
{
	std::vector<WeatherDataPoint> out;
	for (auto itr = in.begin(); itr != in.end(); itr++)
	{
		out.push_back(itr->second);
	}
	return out;
}

inline std::vector<WeatherDataPoint> getDataFromRange(std::vector<WeatherDataPoint> &in, Date start, Date end)
{
	std::vector<WeatherDataPoint> out;
	for (unsigned int i = 0; i < in.size(); i++)
	{
		if (in[i].date >= start && in[i].date <= end)
			out.push_back(in[i]);
	}
	return out;
}

inline std::vector<WeatherDataPoint> getDataFromRangeStrings(std::vector<WeatherDataPoint> &in, std::string start, std::string end)
{
    // Get dates
    Date startDate, endDate;
    try
    {
        // Get start date
        std::vector<std::string> dateParts = splitString(start, '-');
        if (dateParts.size() == 1) // index
        {
            startDate = in.at(stoi(start)).date;
        }
        else if (dateParts.size() == 3) // date
        {
            startDate = Date(stoi(dateParts[0]), stoi(dateParts[1]), stoi(dateParts[2]));
        }
        else
        {
            throw std::exception();
        }

        // Get end date
        dateParts = splitString(end, '-');
        if (dateParts.size() == 1) // index
        {
            endDate = in.at(stoi(end)).date;
        }
        else if (dateParts.size() == 3) // date
        {
            endDate = Date(stoi(dateParts[0]), stoi(dateParts[1]), stoi(dateParts[2]));
        }
        else
        {
            throw std::exception();
        }
    }
    catch (const std::exception &e)
    {
        // invalid inputs, return empty
        return std::vector<WeatherDataPoint>();
    }

    // Get points
    return getDataFromRange(in, startDate, endDate);
}

inline double getAverageOfPoint(std::vector<WeatherDataPoint> points, std::string data)
{
	double result = 0.0;
	int count = 0;

	for (auto point : points)
	{
		if (data == "weather_code")
		{
			result += point.weather_code;
			count++;
		}
		else if (data == "temperature_max")
		{
			result += point.temperature_max;
			count++;
		}
		else if (data == "temperature_min")
		{
			result += point.temperature_min;
			count++;
		}
		else if (data == "precipitation_sum")
		{
			result += point.precipitation_sum;
			count++;
		}
		else if (data == "wind_speed_max")
		{
			result += point.wind_speed_max;
			count++;
		}
		else if (data == "precipitation_probability_max")
		{
			result += point.precipitation_probability_max;
			count++;
		}
	}

	return result / count;
}

inline double getMaxOfPoint(std::vector<WeatherDataPoint> points, std::string data)
{
	double result = -infinity(); // initialize at lowest possible value

	for (auto point : points)
	{
		if (data == "weather_code" && point.weather_code > result)
		{
			result = point.weather_code;
		}
		else if (data == "temperature_max" && point.temperature_max > result)
		{
			result = point.temperature_max;
		}
		else if (data == "temperature_min" && point.temperature_min > result)
		{
			result = point.temperature_min;
		}
		else if (data == "precipitation_sum" && point.precipitation_sum > result)
		{
			result = point.precipitation_sum;
		}
		else if (data == "wind_speed_max" && point.wind_speed_max > result)
		{
			result = point.wind_speed_max;
		}
		else if (data == "precipitation_probability_max" && point.precipitation_probability_max > result)
		{
			result = point.precipitation_probability_max;
		}
	}

	return result;
}

inline double getMinOfPoint(std::vector<WeatherDataPoint> points, std::string data)
{
	double result = infinity(); // initialize at highest possible value

	for (auto point : points)
	{
		if (data == "weather_code" && point.weather_code < result)
		{
			result = point.weather_code;
		}
		else if (data == "temperature_max" && point.temperature_max < result)
		{
			result = point.temperature_max;
		}
		else if (data == "temperature_min" && point.temperature_min < result)
		{
			result = point.temperature_min;
		}
		else if (data == "precipitation_sum" && point.precipitation_sum < result)
		{
			result = point.precipitation_sum;
		}
		else if (data == "wind_speed_max" && point.wind_speed_max < result)
		{
			result = point.wind_speed_max;
		}
		else if (data == "precipitation_probability_max" && point.precipitation_probability_max < result)
		{
			result = point.precipitation_probability_max;
		}
	}

	return result;
}