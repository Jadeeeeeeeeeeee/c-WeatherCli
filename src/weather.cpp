#include "../include/weather.h"
#include "../include/main.h"
#include "fort.hpp"
#include <thread>
#include <curl/curl.h>
#include <nlohmann/json.hpp>
#include <iostream>

size_t WriteCallbackWeather(void* contents, size_t size, size_t nmemb, void* userp) {
    ((std::string*)userp)->append((char*)contents, size * nmemb);
    return size * nmemb;
}

void forcast::getLocation() {
    bool isOn = true;
    forcast WeatherInstence;
    while(isOn){
        std::string apiKey = "e06286f53189a425dec8bd395e8c7678";
        std::cout << "Search a place or type exit: ";
        std::string input;
        std::getline(std::cin, input);

        std::string temp = input;
        std::transform(temp.begin(), temp.end(),
            temp.begin(), ::tolower);
        if (input.empty()) {
            std::cerr << "Error: No input provided. Please enter a valid place name." << std::endl;
            return;
        }
        else if (temp == "exit") {
            exit(0);
        }

        CURL* curl;
        CURLcode res;
        std::string readBuffer;

        curl_global_init(CURL_GLOBAL_DEFAULT);
        curl = curl_easy_init();

        if (curl) {
            std::string link = "https://api.openweathermap.org/geo/1.0/direct?q=" + input + "&limit=1&appid=" + apiKey;
            curl_easy_setopt(curl, CURLOPT_URL, link.c_str());
            curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallbackWeather);
            curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);

            res = curl_easy_perform(curl);
            if (res != CURLE_OK) {
                std::cerr << "Curl request failed: " << curl_easy_strerror(res) << std::endl;
            }
            else if (!readBuffer.empty()) {
                try {
                    auto j = nlohmann::json::parse(readBuffer);

                    if (j.is_array() && !j.empty()) {
                        isOn = false;
                        double lat = j[0]["lat"];
                        double lon = j[0]["lon"];
                        std::string name = j[0]["name"];
                        std::thread currentWeatherThread(&forcast::CurrentWeather, &WeatherInstence,
                            std::to_string(lat), std::to_string(lon), name);
                        std::thread forecastDataThread(&forcast::getForcastData, &WeatherInstence,
                            std::to_string(lat), std::to_string(lon));

                        currentWeatherThread.join();
                        forecastDataThread.join();

                    }
                    else {
                        std::cerr << "Error: Location not found. Please enter a valid place." << std::endl;
                    }
                }
                catch (const nlohmann::json::parse_error& e) {
                    std::cerr << "Error parsing JSON: " << e.what() << std::endl;
                }
            }
            else {
                std::cerr << "Error: No response from server or invalid data received." << std::endl;
            }

            curl_easy_cleanup(curl);
        }

        curl_global_cleanup();
    }
}


void forcast::CurrentWeather(std::string lat, std::string lon, std::string name) {
    std::string apiKey = "e06286f53189a425dec8bd395e8c7678";
    CURL* curl;
    CURLcode res;
    std::string readBuffer;
    curl_global_init(CURL_GLOBAL_DEFAULT);
    curl = curl_easy_init();
    if (curl) {
        std::string link = "https://api.openweathermap.org/data/2.5/weather?lat=" + lat + "&lon=" + lon + "&appid=" + apiKey;
        curl_easy_setopt(curl, CURLOPT_URL, link.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallbackWeather);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);

        res = curl_easy_perform(curl);
        if (res != CURLE_OK) {
            std::cerr << "Curl request failed: " << curl_easy_strerror(res) << std::endl;
        }

        nlohmann::json j = nlohmann::json::parse(readBuffer);
        if (j.contains("main")) {
            std::string city = name;
            std::string temp = std::to_string(static_cast<int>(std::round(j["main"]["temp"].get<int>() - 273.15))) + " C";
            std::string humidity = std::to_string(j["main"]["humidity"].get<int>()) + "%";
            std::string sky = CleanString(j["weather"][0]["main"]);
            ft_table_t* table = ft_create_table();
            ft_set_cell_prop(table, 0, FT_ANY_COLUMN, FT_CPROP_ROW_TYPE, FT_ROW_HEADER);
            ft_write_ln(table, city.c_str(), "  ");
            ft_write_ln(table, "Current temperature: ", temp.c_str());
            ft_write_ln(table, "Current humidity: ", humidity.c_str());
            ft_write_ln(table, "The sky is: ", sky.c_str());
            std::cout << ft_to_string(table) << std::endl;
            ft_destroy_table(table);
          /*  std::cout << "Current temperature: " << j["main"]["temp"] - 273.15 << " C" << std::endl;
            std::cout << "Current humidity: " << j["main"]["humidity"] << std::endl;
            std::cout << "The sky is " << CleanString(j["weather"][0]["main"]) << std::endl;*/
        }
        else {
            throw std::runtime_error("Invalid response: Missing temperature data.");
        }
        curl_easy_cleanup(curl);
    }
    curl_global_cleanup();
}

void forcast::getForcastData(std::string lat, std::string lon) {
    forcast WeatherInstence;
    std::string apiKey = "e06286f53189a425dec8bd395e8c7678";
    CURL* curl;
    CURLcode res;
    std::string readBuffer;
    curl_global_init(CURL_GLOBAL_DEFAULT);
    curl = curl_easy_init();
    if (curl) {
        std::string link = "https://api.openweathermap.org/data/2.5/forecast?lat=" + lat + "&lon=" + lon + "&appid=" + apiKey;
        curl_easy_setopt(curl, CURLOPT_URL, link.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallbackWeather);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);

        res = curl_easy_perform(curl);
        if (res != CURLE_OK) {
            std::cerr << "Curl request failed: " << curl_easy_strerror(res) << std::endl;
        }
        ft_table_t* table = ft_create_table();
        ft_set_cell_prop(table, 0, FT_ANY_COLUMN, FT_CPROP_ROW_TYPE, FT_ROW_HEADER);
        ft_write_ln(table, "Day", "Weather");
        nlohmann::json j = nlohmann::json::parse(readBuffer)["list"];
        for (size_t i = 0; i < j.size(); i = i + 8) {
            std::string temp = std::to_string(static_cast<int>(j[i]["main"]["temp"] - 273.15)) + " C";
            ft_write_ln(table, CleanTime(CleanString(j[i]["dt_txt"])).c_str(), temp.c_str());
            //std::cout << "forcast for " << CleanTime(CleanString(j[i]["dt_txt"])) << ":" << " " << (j[i]["main"]["temp"] - 273.15) << " C " << std::endl;
        }
        std::cout << ft_to_string(table) << std::endl;
        ft_destroy_table(table);
        curl_easy_cleanup(curl);
    }
    curl_global_cleanup();
    WeatherInstence.getLocation();
}

std::string forcast::CleanString(std::string a) {
    std::string result = "";
    for (int i = 0; i < a.size(); i++) {
        if (a[i] != '"') {
            result.push_back(a[i]);
        }
    }
    return result;
}


std::string forcast::CleanTime(std::string a) {
    std::string result = "";
    for (int i = 5; i < a.find(" "); i++) {
        result.push_back(a[i]);
    }
    result.replace(result.find("-"), 1, "/");
    std::swap(result[0], result[3]);
    std::swap(result[1], result[4]);
    return result;
}


