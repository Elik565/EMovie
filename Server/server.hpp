#pragma once

#include <nlohmann/json.hpp>
#include <cpp-httplib/httplib.h>
#include "database.hpp"

// функция преобразования результата sql в json
nlohmann::json pgresult_to_json(PGresult* res);

// функция формирования sql-запроса добавления фильма
std::string form_add_movie_query(const nlohmann::json& body, const std::string& sql_template, httplib::Response& response);

// функция формирования sql-запроса аутентификации
std::string form_auth_query(const nlohmann::json& body, const std::string& sql_template, httplib::Response& response);

// функция формирования sql-запроса на основе шаблона
std::string form_sql_query(const nlohmann::json& body, const std::string& sql_template, httplib::Response& response);

class EMServer {
private:
    httplib::Server server;
    EMDatabase db;

    // метод настройки маршрутов сервера
    void setup_routes();

    // метод обработки GET-запроса
    void handle_get(const std::string& route, const std::string& sql_query);

    // метод обработки POST-запроса
    void handle_post(const std::string& route, const std::string& sql_template);

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

