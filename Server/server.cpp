#include "database.hpp"
#include "server.hpp"
#include <iostream>
#include <cpp-httplib/httplib.h>

using json = nlohmann::json;
using namespace httplib;

json pgresult_to_json(PGresult* res) {
    json json_res = json::array();

    int nrows = PQntuples(res);  // кол-во строк
    int ncols = PQnfields(res);  // кол-во столбцов

    for (int i = 0; i < nrows; i++) {
        json row;
        for (int j = 0; j < ncols; j++) {
            std::string col_name = PQfname(res, j);  // получаем название столбца
            row[col_name] = PQgetvalue(res, i, j);  // получаем значение
        }
        json_res.push_back(row);  // добавляем к результату
    }

    return json_res;
}

std::string get_add_movie_query(const json& body, const std::string& sql_template, Response& response) {
    std::string sql_query = sql_template;

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

    return sql_query;
}

std::string get_auth_query(const json& body, const std::string& sql_template, Response& response) {
    std::string sql_query = sql_template;

    // проверяем наличие всех необходимых полей
    if (!body.contains("login") || !body.contains("password")) {
        response.status = 400;
        response.set_content("{\"error\": \"Ожидаются поля: login (text), password (text)!\"}", "application/json");
        return "";
    }

    // получаем значения полей
    std::string login = body["login"].get<std::string>();
    std::string password = body["password"].get<std::string>();

    // формируем запрос
    sql_query += "'" + login + "' AND password = ";
    sql_query += "'" + password + "'";

    return sql_query;
}

std::string get_sql_query(const json& body, const std::string& sql_template, Response& response) {
    PostTemplates pt;

    try {
        if (sql_template == pt.add_movie) {  // если шаблон добавления фильма
            return get_add_movie_query(body, sql_template, response);
        }

        if (sql_template == pt.auth) {  // если шаблон аутентификации
            return get_auth_query(body, sql_template, response);
        }
    } 
    catch (const std::exception& exc) {  // этот блок обрабатывает исключения
        response.status = 400;
        response.set_content("{\"error\": \"Ошибка при формировании sql-запроса! Проверьте типы данных.\"}", "application/json");
    }

    return "";
}

EMServer::EMServer(const std::string& conn_info) : db(conn_info) {
    setup_routes();
}

EMServer::~EMServer() {
    // т.к. db будет автоматически уничтожен, то вызовется его деструктор
}

void EMServer::setup_routes() {
    GetQueries gq;  // список запросов (для GET)
    PostTemplates pt;  // список шаблонов (для POST)

    handle_get("/show_movie_list", gq.show_movie_list);  // показать список фильмов
    handle_post("/add_movie", pt.add_movie);  // добавить фильм
    handle_post("/auth", pt.auth);  // аутентификация клиента
}

void EMServer::handle_get(const std::string& route, const std::string& sql_query) {
    // лямбда захватывает поля текущего класса и sql-запрос
    // request и response - это параметры лямбды
    server.Get(route, [this, sql_query](const Request& request, Response& response) {
        std::cout << "Получен GET-запрос с путем: " << request.path << std::endl;

        PGresult* sql_result = db.execute_query(sql_query);

        if (PQresultStatus(sql_result) != PGRES_TUPLES_OK) {
            response.status = 500;
            response.set_content("{\"error\":\"" + db.get_sql_error() + "\"}", "application/json");
            PQclear(sql_result);
            return;
        }

        json json_res = pgresult_to_json(sql_result);  // преобразуем результат в json
        PQclear(sql_result);  // очищаем память после использования результата sql запроса
        response.set_content(json_res.dump(), "application/json");  // устанавливаем содержимое HTTP ответа в виде json
    });
}

void EMServer::handle_post(const std::string& route, const std::string& sql_template) {
    server.Post(route, [this, sql_template](const Request& request, Response& response) {
        std::cout << "Получен POST-запрос с путем: " << request.path << std::endl;

        try {
            auto body = json::parse(request.body);  // парсим json-тело запроса
            std::string sql_query = get_sql_query(body, sql_template, response);  // формируем полный sql-запрос

            if (sql_query.empty()) {
                return;
            }

            PGresult* result = db.execute_query(sql_query);  // выполняем запрос

            if (PQresultStatus(result) == PGRES_TUPLES_OK) {
                int rows = PQntuples(result);
                if (rows == 0) {  // если вернулось 0 строк
                    response.status = 404;
                    response.set_content("{\"error\": \"Пользователь не найден!\"}", "application/json");
                    PQclear(result);
                    return;
                }
            }
            // если есть ошибка при выполнении sql-запроса
            else if (PQresultStatus(result) != PGRES_COMMAND_OK) {
                std::string error_msg = db.get_sql_error();  // получаем текст ошибки

                response.status = 500;
                response.set_content("{\"error\": \"" + error_msg + "\"}", "application/json");
                PQclear(result);
                return;
            }

            PQclear(result);
            response.status = 200;
        }   
        catch (const std::exception& exc) {  // этот блок обрабатывает исключения
            // ошибка при парсинге json
            response.status = 400;
            response.set_content("{\"error\": \"Некорректный JSON!\"}", "application/json");
        }
    });
}

void EMServer::start() {
    std::cout << "Сервер запущен на http://localhost:8080\n";
    std::cout << "Нажмите Ctrl+C для завершения работы сервера...\n";

    if (!server.listen("0.0.0.0", 8080)) {  // начинаем слушать запросы
        std::cerr << "Не удалось запустить сервер (возможно, порт занят)\n";
    }
}

void EMServer::stop() {
    server.stop();
}