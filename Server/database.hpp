#pragma once

#include <libpq-fe.h>
#include <string>
#include <nlohmann/json.hpp>

// структура фильма
struct Movie {
    int id;
    std::string title;
    int year;
};

// список запросов
struct GetQueries {
    std::string show_movie_list = "SELECT title, year FROM movies LIMIT 10";
};

struct PostTemplates {
    std::string add_movie = "INSERT INTO movies (id, title, year) VALUES (";
};

// функция для подключения к базе данных
PGconn* connect_to_db(const std::string& conn_info);

// функция преобразования PGresult в json
nlohmann::json pgresult_to_json(PGresult* res);


