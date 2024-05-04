#pragma once

#include <string>

/**
 * @brief an HTTP client to obtain data from the ESP32
 * 
 */
class HTTPClient
{
public:
    HTTPClient(std::string esp_server_ip);
    ~HTTPClient();

    /**
     * @brief get the data from the ESP32
     * 
     * @return std::string the data from the ESP32
     */
    std::string get_data();

private:
    std::string esp_server_ip;
};
