#pragma once

#include <cpp-httplib/httplib.h>
#include <nlohmann/json.hpp>
#include <iostream>

// функция проверки ответа от сервера
nlohmann::json check_http_result(httplib::Result& result);

class BaseClient {
protected:
    httplib::Client client;  // http-клиент для общения с сервером
    httplib::Headers headers;  // заголовки для http-запросов

public:
    // конструктор
    BaseClient(const std::string& host, int port)
        : client(host, port) {}

    // метод отправки GET-запроса
    nlohmann::json send_get(const std::string& route);

    // метод отправки POST-запроса
    nlohmann::json send_post(const std::string& route, const nlohmann::json& body);
};
