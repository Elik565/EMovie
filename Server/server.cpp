#include "database.hpp"
#include "server.hpp"
#include <iostream>
#include <cpp-httplib/httplib.h>
#include <random>

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

std::string generate_token() {
    const std::string chars = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> distrib(0, chars.size() - 1);

    std::string token;
    for (size_t i = 0; i < 16; ++i) {
        token += chars[distrib(gen)];
    }

    return token;
}


EMServer::EMServer(const std::string& conn_info) : db(conn_info) {
    setup_routes();
}

EMServer::~EMServer() {
    // т.к. db будет автоматически уничтожен, то вызовется его деструктор
}

void EMServer::set_error(httplib::Response& response, const int status, const std::string& message) {
    response.status = status;
    response.set_content("{\"error\": \"" + message + "\"}", "application/json");
}

void EMServer::setup_routes() {
    Queries q;

    handle_get("/show_movie_list", q.show_movie_list);  // показать список фильмов
    handle_post("/add_movie");  // добавить фильм
    handle_post("/auth");  // аутентификация клиента
}

void EMServer::handle_get(const std::string& route, const std::string& sql_query) {
    // лямбда захватывает поля текущего класса и sql-запрос
    // request и response - это параметры лямбды
    server.Get(route, [this, sql_query](const Request& request, Response& response) {
        std::cout << "Получен GET-запрос с путем: " << request.path << std::endl;

        PGresult* sql_result = db.execute_query(sql_query);

        if (PQresultStatus(sql_result) != PGRES_TUPLES_OK) {
            set_error(response, 500, db.get_sql_error());
            PQclear(sql_result);
            return;
        }

        json json_res = pgresult_to_json(sql_result);  // преобразуем результат в json
        PQclear(sql_result);  // очищаем память после использования результата sql запроса
        response.set_content(json_res.dump(), "application/json");  // устанавливаем содержимое HTTP ответа в виде json
    });
}

PGresult* EMServer::handle_add_movie(const json& body, Response& response) {
    // проверяем наличие всех необходимых полей
    if (!body.contains("id") || !body.contains("title") || !body.contains("year")) {
        set_error(response, 400, "Ожидаются поля: id (int), title (text), year (int)!");
        return nullptr;
    }

    try {
        // получаем значения полей
        int id = body["id"].get<int>();
        std::string title = body["title"].get<std::string>();
        int year = body["year"].get<int>();

        // формируем sql-запрос
        std::string sql_query = "INSERT INTO movies (id, title, year) VALUES (" + std::to_string(id) + ", ";
        sql_query += "'" + title + "', ";
        sql_query += std::to_string(year) + ")";

        return db.execute_query(sql_query);
    }
    catch (const std::exception& exc) {
        set_error(response, 400, "Ошибка при формировании sql-запроса! Проверьте типы данных.");
        return nullptr;
    }
}

PGresult* EMServer::handle_auth(const json& body, Response& response) {
    // проверяем наличие всех необходимых полей
    if (!body.contains("login") || !body.contains("password")) {
        set_error(response, 400, "Ожидаются поля: login (text), password (text)!");
        return nullptr;
    }

    try {
        // получаем значения полей
        std::string login = body["login"].get<std::string>();
        std::string password = body["password"].get<std::string>();

        // формируем запрос
        std::string sql_query = "SELECT 1 FROM users WHERE username = ";
        sql_query += "'" + login + "' AND password = ";
        sql_query += "'" + password + "'";

        PGresult* sql_result = db.execute_query(sql_query);  // выполняем запрос

        if (PQresultStatus(sql_result) == PGRES_TUPLES_OK) {
            int rows = PQntuples(sql_result);
            if (rows == 0) {  // если вернулось 0 строк
                set_error(response, 404, "Пользователь не найден!");
                PQclear(sql_result);
                return nullptr;
            }
        }

        // если пользователь найден
        std::string token = generate_token();
        json json_response = { {"message", "Успешная авторизация!"}, {"token", token} };
        response.set_content(json_response.dump(), "application/json");
        response.status = 200;

        return nullptr;  // уже отправили ответ
    }
    catch (const std::exception& exc) {
        set_error(response, 400, "Ошибка при формировании sql-запроса! Проверьте типы данных");
        return nullptr;
    }
}

void EMServer::handle_post(const std::string& route) {
    server.Post(route, [this, route](const Request& request, Response& response) {
        std::cout << "Получен POST-запрос с путем: " << request.path << std::endl;

        try {
            auto body = json::parse(request.body);  // парсим json-тело запроса
            PGresult* sql_result = nullptr;
            
            if (route == "/add_movie") {
                sql_result = handle_add_movie(body, response);
            }
            else if (route == "/auth") {
                sql_result = handle_auth(body, response);
            }
            else {
                set_error(response, 404, "Маршрут не найден!");
                return;
            }

            if (!sql_result) {
                return;
            }

            if (PQresultStatus(sql_result) != PGRES_COMMAND_OK) {
                set_error(response, 500, db.get_sql_error());
                PQclear(sql_result);
                return;
            }

            PQclear(sql_result);
            response.status = 200;
        }   
        catch (const std::exception& exc) {  // этот блок обрабатывает исключения
            // ошибка при парсинге json
            set_error(response, 400, "Некорректный JSON!");
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