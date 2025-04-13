#include "database.hpp"
#include "server.hpp"

#include <iostream>
#include <string>
#include <cpp-httplib/httplib.h>

using json = nlohmann::json;
using namespace httplib;

void handle_get(httplib::Server& server, PGconn* conn, const std::string& route, const std::string& sql_query) {
    server.Get(route, [conn, sql_query](const Request& request, Response& response) {
        std::cout << "Получен GET-запрос с путем: " << request.path << std::endl;

        PGresult* sql_result = execute_query(conn, sql_query);

        // если ошибка выполнения sql запроса
        if (!sql_result) {
            response.status = 500;
            response.set_content("{\"error\":\"Ошибка выполнения запроса\"}", "application/json");
            return;
        }

        json json_res = pgresult_to_json(sql_result);  // преобразуем результат в json

        PQclear(sql_result);  // очищаем память после использования результата sql запроса
        
        response.set_content(json_res.dump(), "application/json");  // устанавливаем содержимое HTTP ответа в виде json
    });
}

void setup_routes(httplib::Server& server, PGconn* conn) {
    Queries queries;  // список запросов

    handle_get(server, conn, "/show_movie_list", queries.show_movie_list);  // показать список фильмов
}

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

    Server server;  // запускаем сервер

    setup_routes(server, conn);  // настраиваем маршруты
    std::cout << "Маршруты сервера настроены!\n";

    std::cout << "Сервер запущен на http://localhost:8080\n";
    server.listen("0.0.0.0", 8080);  // слушаем запросы

    PQfinish(conn);
    std::cout << "Соединение с базой данных завершено!\n";

    return 0;
}