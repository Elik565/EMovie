#pragma once

#include <httplib.h>
#include <nlohmann/json.hpp>
#include <iostream>

class BaseClient {
protected:
    httplib::Client client;
    httplib::Headers headers;

    nlohmann::json check_http_result(httplib::Result& result);

public:
    BaseClient(const std::string& host, int port = 80)
        : client(host, port) {}

    nlohmann::json send_get(const std::string& route);
    nlohmann::json send_post(const std::string& route, const nlohmann::json& body = {});
};
