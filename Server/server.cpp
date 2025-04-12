#include "database.hpp"
#include <iostream>
#include <string>

int main(int argc, char* argv[]) {
    if (argc != 3) {
        std::cerr << "Неправильно переданные параметры (логин, пароль)!\n";
        exit(1); 
    }

    std::string conn_info = "dbname=EMovieDB host=localhost user=" + std::string(argv[1]) + " password=" + argv[2];

    PGconn* conn = connect_to_db(conn_info);  // подключаемся к бд
    if (!conn) {
        return 1;
    }

    Queries queries;  // список запросов

    PGresult* res = execute_query(conn, queries.show_movie_list);

    return 0;
}