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

        PGresult* sql_result = PQexec(conn, sql_query.c_str());

        if (!sql_result || PQresultStatus(sql_result) != PGRES_TUPLES_OK) {
            response.status = 500;
            response.set_content("{\"error\":\"Ошибка выполнения запроса\"}", "application/json");
            if (sql_result) PQclear(sql_result);
            return;
        }

        json json_res = pgresult_to_json(sql_result);  // преобразуем результат в json

        PQclear(sql_result);  // очищаем память после использования результата sql запроса
        
        response.set_content(json_res.dump(), "application/json");  // устанавливаем содержимое HTTP ответа в виде json
    });
}

std::string get_sql_query(const json& body, const std::string& sql_template, Response& response) {
    std::string sql_query = sql_template;
    PostTemplates pt;

    try {
        if (sql_template == pt.add_movie) {// если шаблон добавления фильма
            // проверяем наличие всех необходимых полей
            if (!body.contains("id") || !body.contains("title") || !body.contains("year")) {
                response.status = 400;
                response.set_content("{\"error\": \"Ожидаются поля: id (int), title (text), year (int)!\"}", "application/json");
                return "";
            }

            // получаем значения полей
            int id = body["id"].get<int>();
            std::string title = body["title"].get<std::string>();
            int year = body["year"].get<int>();

            // формируем запрос
            sql_query += std::to_string(id) + ", ";
            sql_query += "'" + title + "', ";
            sql_query += std::to_string(year) + ")";
        }
    } 
    catch (const std::exception& exc) {  // этот блок обрабатывает исключения
        response.status = 400;
        response.set_content("{\"error\": \"Ошибка при формировании sql-запроса. Проверьте правильность типов данных!\"}", "application/json");
        return "";
    }

    return sql_query;
}

void handle_post(httplib::Server& server, PGconn* conn, const std::string& route, const std::string& sql_template) {
    server.Post(route, [conn, sql_template](const Request& request, Response& response) {
        std::cout << "Получен POST-запрос с путем: " << request.path << std::endl;

        try {
            auto body = json::parse(request.body);  // парсим json-тело запроса
            std::string sql_query = get_sql_query(body, sql_template, response);  // формируем полный sql-запрос

            if (sql_query.empty()) {
                return;
            }

            PGresult* result = PQexec(conn, sql_query.c_str());  // выполняем запрос

            // если есть ошибка при выполнении sql-запроса
            if (PQresultStatus(result) != PGRES_COMMAND_OK) {
                std::string error_msg = PQerrorMessage(conn);  // получаем текст ошибки

                response.status = 500;
                response.set_content("{\"error\": \"" + error_msg + "\"}", "application/json");
                return;
            }

            PQclear(result);
            response.status = 200;
            response.set_content("{\"success\": \"Фильм добавлен!\"}", "application/json");
        }   
        catch (const std::exception& exc) {  // этот блок обрабатывает исключения
            // ошибка при парсинге json
            response.status = 400;
            response.set_content("{\"error\": \"Некорректный JSON!\"}", "application/json");
        }
    });
}

void setup_routes(httplib::Server& server, PGconn* conn) {
    GetQueries gq;  // список запросов (для GET)
    PostTemplates pt;  // список шаблонов (для POST)

    handle_get(server, conn, "/show_movie_list", gq.show_movie_list);  // показать список фильмов
    handle_post(server, conn, "/add_movie", pt.add_movie);  // добавить фильм
}