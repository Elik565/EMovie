#pragma once

#include <cpp-httplib/httplib.h>

class EMClient {
private:
    std::string login;
    std::string password;
    bool authenticated = false;  // флаг, авторизован ли пользователь
    httplib::Client client;  // http-клиент для общения с сервером

public:
    // конструктор
    EMClient(const std::string& host, const int port) : client(host, port) {};

    // деструктор
    ~EMClient();

    // метод ввода логина и пароля (или запрос на регистрацию)
    void enter_login_password();

    // метод аутентификации клиента
    bool authentication();
};




