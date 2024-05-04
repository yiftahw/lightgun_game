#include "http_client.h"

#include <cpr/api.h>
#include <iostream>


HTTPClient::HTTPClient(std::string esp_server_ip)
    : esp_server_ip(esp_server_ip)
{
}

HTTPClient::~HTTPClient()
{
}

std::string HTTPClient::get_data()
{
    // try fetching from the esp
    cpr::Response r = cpr::Get(cpr::Url{esp_server_ip});
    if (r.status_code != 200)
    {
        std::cerr << "Failed to fetch data from " << esp_server_ip << std::endl;
        return "[(1023,1023),(1023,1023),(1023,1023),(1023,1023)]"; // return out of range values
    }
    return r.text;
}
