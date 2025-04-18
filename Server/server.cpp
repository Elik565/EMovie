#include "database.hpp"
#include "server.hpp"
#include <iostream>
#include <cpp-httplib/httplib.h>
#include <random>
#include <string>

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

void set_error(httplib::Response& response, const int status, const std::string& message) {
    response.status = status;
    response.set_content("{\"error\": \"" + message + "\"}", "application/json");
}

std::string get_token_from_request(const Request& request, Response& response) {
    std::string auth_header = request.get_header_value("Authorization");

    // если нет типа токена Bearer
    if (auth_header.substr(0, 7) != "Bearer ") {
        set_error(response, 400, "Некорректный заголовок авторизации (не передан токен)!");
        return "";
    }

    return auth_header.substr(7);  // "Bearer " — 7 символов
}

PGresult* safe_sql_query(Response& response, std::function<PGresult*()> func) {
    try {
        return func();
    } catch (...) {
        set_error(response, 400, "Ошибка при формировании sql-запроса!");
        return nullptr;
    }
}


EMServer::EMServer(const std::string& conn_info) : db(conn_info) {
    setup_routes();  // настраиваем маршруты
}

EMServer::~EMServer() {
    // т.к. db будет автоматически уничтожен, то вызовется его деструктор
}

void EMServer::setup_routes() {
    Queries q;

    handle_get("/movie_list", q.movie_list);  // показать список фильмов
    handle_post("/reg");  // регистрация нового клиента
    handle_post("/add_movie");  // добавить фильм
    handle_post("/auth");  // авторизация клиента
    handle_post("/logout");  // завершении сессии клиента
}

bool EMServer::is_authorized(const Request& request, Response& response) {
    std::string token = get_token_from_request(request, response);

    if (token == "") {
        return false;  // уже отправили ответ
    }
    
    if (sessions.find(token) == sessions.end()) {  // поиск токена в текущих сессиях
        set_error(response, 401, "Пользователь не авторизован!");
        return false;
    }

    return true;
}

bool EMServer::is_admin(const Request& request, Response& response) {
    std::string token = get_token_from_request(request, response);

    if (token == "") {
        return false;  // уже отправили ответ
    }

    if (sessions[token].is_admin == true) {
        return true;
    }
    
    set_error(response, 401, "Пользователь не является администратором!");
    return false;
}

void EMServer::handle_get(const std::string& route, const std::string& sql_query) {
    // лямбда захватывает поля текущего класса и sql-запрос
    // request и response - это параметры лямбды
    server.Get(route, [this, sql_query](const Request& request, Response& response) {
        std::cout << "Получен GET-запрос с путем: " << request.path << std::endl;

        // проверка, авторизован ли клиент
        if (!is_authorized(request, response)) {
            return;
        }

        PGresult* sql_result = db.execute_query(sql_query);

        // проверка на ошибки при выполнении sql-запроса
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

PGresult* EMServer::handle_reg(const json& body, Response& response) {
    // проверяем наличие всех необходимых полей
    if (!body.contains("login") || !body.contains("password")) {
        set_error(response, 400, "Ожидаются поля: login (text), password (text)!");
        return nullptr;
    }

    return safe_sql_query(response, [&] {
        // получаем значения полей
        std::string login = body["login"].get<std::string>();
        std::string password = body["password"].get<std::string>();

        // формируем запрос
        std::string sql_query = "INSERT INTO users (username, password) VALUES (";
        sql_query += "'" + login + "', ";
        sql_query += "'" + password + "')";

        return db.execute_query(sql_query);  // выполняем запрос
    });
}

PGresult* EMServer::handle_auth(const json& body, Response& response) {
    // проверяем наличие всех необходимых полей
    if (!body.contains("login") || !body.contains("password")) {
        set_error(response, 400, "Ожидаются поля: login (text), password (text)!");
        return nullptr;
    }

    return safe_sql_query(response, [&]() -> PGresult* {
        // получаем значения полей
        std::string login = body["login"].get<std::string>();
        std::string password = body["password"].get<std::string>();

        // формируем запрос
        std::string sql_query = "SELECT can_modify FROM users WHERE username = ";
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
            else {  // если пользователь найден
                std::string token = generate_token();
                sessions[token].login = login;
                sessions[token].is_admin = (PQgetvalue(sql_result, 0, 0)[0] == 't');

                PQclear(sql_result);
                json json_response = { {"message", "Успешная авторизация!"}, {"token", token}, {"is_admin", sessions[token].is_admin} };
                response.set_content(json_response.dump(), "application/json");
                response.status = 200;
                return nullptr;
            }
        }

        return sql_result;
    });
}

PGresult* EMServer::handle_add_movie(const json& body, Response& response) {
    // проверяем наличие всех необходимых полей
    if (!body.contains("id") || !body.contains("title") || !body.contains("year")) {
        set_error(response, 400, "Ожидаются поля: id (int), title (text), year (int)!");
        return nullptr;
    }

    return safe_sql_query(response, [&] {
        // получаем значения полей
        int id = std::stoi(body["id"].get<std::string>());
        std::string title = body["title"].get<std::string>();
        int year = std::stoi(body["year"].get<std::string>());

        // формируем sql-запрос
        std::string sql_query = "INSERT INTO movies (id, title, year) VALUES (" + std::to_string(id) + ", ";
        sql_query += "'" + title + "', ";
        sql_query += std::to_string(year) + ")";

        return db.execute_query(sql_query);
    });
}

void EMServer::handle_logout(const Request& request, Response& response) {
    std::string token = get_token_from_request(request, response);

    if (token == "") {
        return;  // уже отправили ответ
    }

    sessions.erase(token);  // удаляем сессию
    response.status = 200;
    response.set_content("{\"message\": \"Сессия завершена!\"}", "application/json");
}

void EMServer::handle_post(const std::string& route) {
    server.Post(route, [this, route](const Request& request, Response& response) {
        std::cout << "Получен POST-запрос с путем: " << route << std::endl;

        if (route != "/auth" && route != "/reg") {  // если не авторизация и не регистрация
            // проверка, авторизован ли клиент
            if (!is_authorized(request, response)) {
                return;
            }
        }
        
        try {
            PGresult* sql_result = nullptr;
            auto body = request.body.empty() ? json{} : json::parse(request.body);
            
            if (route == "/reg") {
                sql_result = handle_reg(body, response);
            }
            else if (route == "/auth") {
                sql_result = handle_auth(body, response);
            }
            else if (route == "/add_movie") {
                if (!is_admin(request, response)) {
                    return;
                }
                sql_result = handle_add_movie(body, response);
            }
            else if (route == "/logout") {
                handle_logout(request, response);
            }

            // если уже отправили ответ
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
            response.set_content("{\"message\": \"Успешно!\"}", "application/json");
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