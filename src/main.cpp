#include <iostream>
#include <winsock2.h>
#include <Windows.h>
#include <WS2tcpip.h>
#include <curl/curl.h>
#include <nlohmann/json.hpp>
#include <string>
#include <thread>
#include "../include/weather.h"
#include "../include/main.h"


size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp) {
    ((std::string*)userp)->append((char*)contents, size * nmemb);
    return size * nmemb;
}


int main() {
    Main MainInstence;
    std::cout << "Welcome to Jade's Weather App" << "\n";

    MainInstence.ip = MainInstence.GetIp();
    MainInstence.GetCurrentLocation();
}

std::string Main::GetIp() {
    CURL* curl;
    CURLcode res;
    std::string ip_address;

    curl_global_init(CURL_GLOBAL_DEFAULT);
    curl = curl_easy_init();

    if (curl) {
        curl_easy_setopt(curl, CURLOPT_URL, "https://api.ipify.org");
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &ip_address);

        res = curl_easy_perform(curl);

        if (res != CURLE_OK) {
            std::cerr << "Curl request failed: " << curl_easy_strerror(res) << std::endl;
        }

        curl_easy_cleanup(curl);
    }

    curl_global_cleanup();
    return ip_address;
}

void Main::GetCurrentLocation() {
    forcast WeatherInstence; 
    CURL* curl;
    CURLcode res;
    std::string readBuffer;

    curl_global_init(CURL_GLOBAL_DEFAULT);
    curl = curl_easy_init();
    if (curl) {
        std::string link = "http://ip-api.com/json/" + ip;
        curl_easy_setopt(curl, CURLOPT_URL, link.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);

        res = curl_easy_perform(curl);

        if (res != CURLE_OK) {
            std::cerr << "Curl request failed: " << curl_easy_strerror(res) << std::endl;
        }

        curl_easy_cleanup(curl);
    }
    nlohmann::json j = nlohmann::json::parse(readBuffer);
    if (j["status"] == "success") {
        float lat = j["lat"];
        float lon = j["lon"];
        std::string place = j["regionName"];
        std::string latString = std::to_string(lat);
        std::string lonString = std::to_string(lon);
        //WeatherInstence.CurrentWeather(latString, lonString, place);
        //WeatherInstence.getForcastData(latString, lonString);
        std::thread currentWeatherThread(&forcast::CurrentWeather, &WeatherInstence, latString, lonString, place);
        std::thread forecastDataThread(&forcast::getForcastData, &WeatherInstence, latString, lonString);

        currentWeatherThread.join();
        forecastDataThread.join();
    }
}
