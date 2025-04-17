#pragma once

#include <cpp-httplib/httplib.h>
#include <nlohmann/json.hpp>

class EMClient {
private:
    std::string login;
    std::string password;
    std::string token;
    httplib::Client client;  // http-клиент для общения с сервером
    httplib::Headers headers;  // заголовки для запросов
    bool authenticated = false;  // флаг, авторизован ли пользователь

    // метод отправки GET-запроса
    nlohmann::json send_get(const std::string& route);

public:
    // конструктор
    EMClient(const std::string& host, const int port) : client(host, port) {};

    // деструктор
    ~EMClient() {};

    // метод ввода логина и пароля (или запрос на регистрацию)
    void enter_login_password();

    // метод авторизации клиента
    bool authorization();

    void show_movie_list();
};




