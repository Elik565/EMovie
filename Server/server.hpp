#pragma once

#include <nlohmann/json.hpp>
#include <cpp-httplib/httplib.h>
#include "database.hpp"

// функция преобразования результата sql в json
nlohmann::json pgresult_to_json(PGresult* res);

// функция генерации токена
std::string generate_token();

class EMServer {
private:
    httplib::Server server;
    EMDatabase db;
    std::unordered_map<std::string, std::string> sessions;

    // метод установки сообщения об ошибке
    void set_error(httplib::Response& response, const int status, const std::string& message);

    // метод настройки маршрутов сервера
    void setup_routes();

    // метод обработки GET-запроса
    void handle_get(const std::string& route, const std::string& sql_query);

    // метод обработки запроса добавления фильма
    PGresult* handle_add_movie(const nlohmann::json& body, httplib::Response& response);

    // метод обработки запроса аутентификации
    PGresult* handle_auth(const nlohmann::json& body, httplib::Response& response);

    // метод обработки POST-запроса
    void handle_post(const std::string& route);

    std::string create_session(const std::string& login);
    bool is_authorized(const std::string& token) const;

public:
    // конструктор
    EMServer(const std::string& conn_info);

    // деструктор
    ~EMServer();

    // метод запуска сервера
    void start();

    // метод остановки сервера
    void stop();
};

