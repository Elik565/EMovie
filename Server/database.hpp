#pragma once

#include <libpq-fe.h>
#include <string>

struct Movie {
    int id;
    std::string title;
    int year;
};

// функция для подключения к базе данных
PGconn* connect_to_db(const std::string& conn_info);