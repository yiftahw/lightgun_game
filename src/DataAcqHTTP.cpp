#include "DataAcqHTTP.h"

#include <cpr/api.h>
#include <iostream>


DataAcqHTTP::DataAcqHTTP(std::string esp_server_ip)
    : esp_server_ip(esp_server_ip)
{
}

DataAcqHTTP::~DataAcqHTTP()
{
}

Snapshot DataAcqHTTP::get()
{
    // try fetching from the esp
    constexpr Snapshot empty{Point{1023, 1023}, Point{1023, 1023}, Point{1023, 1023}, Point{1023, 1023}}; // invalid snapshot
    
    cpr::Response r = cpr::Get(cpr::Url{esp_server_ip});
    if (r.status_code != 200)
    {
        std::cerr << "Failed to fetch data from " << esp_server_ip << std::endl;
        return empty;
    }
    return snapshot_from_string(r.text);
}
