#include "database.hpp"
#include "server.hpp"

#include <iostream>
#include <string>
#include <cpp-httplib/httplib.h>
#include <signal.h>

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