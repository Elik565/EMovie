#pragma once

#include <libpq-fe.h>
#include <string>

// список запросов
struct Queries {
    std::string movie_list = "SELECT title, year FROM movies LIMIT 10";
};

class EMDatabase {
private:
    PGconn* conn;

public:
    // конструктор
    EMDatabase(const std::string& conn_info);

    // деструктор
    ~EMDatabase();

    // метод выполнения sql-запроса
    PGresult* execute_query(const std::string& sql_query) const;

    // метод получения ошибки при выполнении sql-запроса
    std::string get_sql_error() const;
};


