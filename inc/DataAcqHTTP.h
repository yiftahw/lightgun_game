#pragma once

#include <string>
#include "IDataAcq.h"

/**
 * @brief an HTTP client to obtain data from the ESP32
 * 
 */
class DataAcqHTTP : public IDataAcq
{
public:
    DataAcqHTTP(const std::string &esp_server_ip);
    ~DataAcqHTTP();

    Snapshot get() override;

private:
    std::string esp_server_ip;
};
