#pragma once

#include <cpp-httplib/httplib.h>
#include <nlohmann/json.hpp>

nlohmann::json check_http_result(httplib::Result& result);

class EMClient {
private:
    std::string login;
    std::string password;
    std::string token;
    httplib::Client client;  // http-клиент для общения с сервером
    httplib::Headers headers;  // заголовки для http-запросов
    bool is_admin = false;  // флаг, является ли клиент администратором

    // метод отправки GET-запроса
    nlohmann::json send_get(const std::string& route);

    // метод отправки POST-запроса
    nlohmann::json send_post(const std::string& route, const nlohmann::json& body);

    // метод завершения сессии
    void logout();

public:
    // конструктор
    EMClient(const std::string& host, const int port) : client(host, port) {};  // инициализирует поле client

    // деструктор
    ~EMClient();

    // метод ввода логина и пароля (или запрос на регистрацию)
    void enter_login_password();

    // метод авторизации клиента
    bool authorization();

    // метод отображения списка фильмов
    void show_movie_list();
};




