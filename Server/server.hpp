#pragma once

#include <cpp-httplib/httplib.h>
#include "database.hpp"

// обработчик сигнала SIGINT
void sigint_handler(int sigint);

// функция обработки GET-запроса
void handle_get(httplib::Server& server, PGconn* conn, const std::string& route, const std::string& query);

// функция настройки маршрутов сервера
void setup_routes(httplib::Server& server, PGconn* conn);