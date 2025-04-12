#pragma once

#include <libpq-fe.h>
#include <string>

// структура фильма
struct Movie {
    int id;
    std::string title;
    int year;
};

// список запросов
struct Queries {
    std::string show_movie_list = "SELECT title, year FROM movies LIMIT 10";
};

// функция для подключения к базе данных
PGconn* connect_to_db(const std::string& conn_info);

// функция выполнения sql-запроса
PGresult* execute_query(PGconn* conn, const std::string& query);

// функция закрытия соединения с базой данных
void close_connection(PGconn* conn, PGresult* res);

