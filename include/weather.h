#ifndef FORCAST_H
#define FORCAST_H

#include <string>

class forcast {
public:
	void CurrentWeather(std::string lat, std::string lon, std::string name);
	void getForcastData(std::string lat, std::string lon);
	void getLocation();
	std::string CleanString(std::string a);
	std::string CleanTime(std::string a);
	void printForcastData();
};

#endif