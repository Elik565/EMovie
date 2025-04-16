#pragma once

#include <cpp-httplib/httplib.h>

class EMClient {
private:
    std::string login;
    std::string password;
    std::string token;
    httplib::Client client;  // http-клиент для общения с сервером
    httplib::Headers headers;  // заголовки для запросов
    bool authenticated = false;  // флаг, авторизован ли пользователь

public:
    // конструктор
    EMClient(const std::string& host, const int port) : client(host, port) {};

    // деструктор
    ~EMClient() {};

    // метод ввода логина и пароля (или запрос на регистрацию)
    void enter_login_password();

    // метод авторизации клиента
    bool authorization();
};




