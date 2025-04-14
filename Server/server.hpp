#pragma once

#include <cpp-httplib/httplib.h>
#include "database.hpp"

// функция обработки GET-запроса
void handle_get(httplib::Server& server, PGconn* conn, const std::string& route, const std::string& sql_query);

// функция формирования sql-запроса добавления фильма
std::string get_add_movie_query(const nlohmann::json& body, const std::string& sql_template, httplib::Response& response);

// функция формирования sql-запроса аутентификации
std::string get_auth_query(const nlohmann::json& body, const std::string& sql_template, httplib::Response& response);

// функция формирования sql-запроса на основе шаблона
std::string get_sql_query(const nlohmann::json& body, const std::string& sql_template, httplib::Response& response);

// функция обработки POST-запроса
void handle_post(httplib::Server& server, PGconn* conn, const std::string& route, const std::string& sql_template);

// функция настройки маршрутов сервера
void setup_routes(httplib::Server& server, PGconn* conn);