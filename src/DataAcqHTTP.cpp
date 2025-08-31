#include "DataAcqHTTP.h"

#include <cpr/api.h>
#include <iostream>


DataAcqHTTP::DataAcqHTTP(const std::string &esp_server_ip)
    : esp_server_ip(esp_server_ip)
{
}

DataAcqHTTP::~DataAcqHTTP() = default;

Snapshot DataAcqHTTP::get()
{
    // try fetching from the esp32 server
    cpr::Response r = cpr::Get(cpr::Url{esp_server_ip});
    if (r.status_code != 200)
    {
        std::cerr << "Failed to fetch data from " << esp_server_ip << std::endl;
        return Snapshot::invalid();
    }
    return snapshot_from_string(r.text);
}
