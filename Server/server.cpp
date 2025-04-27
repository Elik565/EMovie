#include "database.hpp"
#include "server.hpp"
#include <iostream>
#include <cpp-httplib/httplib.h>
#include <random>
#include <string>
#include <fstream>

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

    if (auth_header.empty()) {
        auth_header = request.get_header_value("Referer");
    }

    // если нет типа токена Bearer
    if (auth_header.substr(0, 7) != "Bearer ") {
        set_error(response, 400, "Некорректный заголовок проверки авторизации (не передан токен)!");
        return "";
    }

    return auth_header.substr(7);  // "Bearer " — 7 символов
}

PGresult* safe_sql_query(Response& response, std::function<PGresult*()> func) {
    // принимает на вход функцию, которая возвращает PGresult*
    try {
        return func();
    } catch (...) {
        set_error(response, 400, "Ошибка при формировании sql-запроса!");
        return nullptr;
    }
}

void send_hls_playlist(const std::string& filepath, httplib::Response& response) {
    std::ifstream fin(filepath);
    if (!fin) {
        set_error(response, 404, "Плейлист HLS не найден!");
        return;
    }

    std::stringstream playlist_stream;
    playlist_stream << fin.rdbuf();  // записываем буфер данных из потока файла в строковый поток
    fin.close();

    response.status = 200;
    response.set_header("Content-Type", "application/vnd.apple.mpegurl");
    response.body = playlist_stream.str();
}


EMServer::EMServer(const std::string& conn_info) : db(conn_info) {
    // лямбда захватывает поля текущего класса и sql-запрос
    // request и response - это параметры лямбды
    server.Get(R"(/.*)", [this](const Request& request, Response& response) {
        handle_get(request, response);
    });

    server.Post(R"(/.*)", [this](const Request& request, Response& response) {
        handle_post(request, response);
    });
}

EMServer::~EMServer() {
    // т.к. db будет автоматически уничтожен, то вызовется его деструктор
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

PGresult* EMServer::handle_movie_list(const Request& request, Response& response) {
    Queries q;  // список sql-запросов
    return db.execute_query(q.movie_list);
}

std::string EMServer::get_playlist_filepath(const std::string& title, Response& response) {
    std::string sql_query = "SELECT filepath FROM movies WHERE title LIKE '%" + title + "%'";
    PGresult* sql_result = db.execute_query(sql_query);  // выполняем sql-запрос

    // проверяем на наличие ошибок и на нахождение фильма
    if (!sql_result || PQntuples(sql_result) == 0) {
        PQclear(sql_result);
        set_error(response, 404, "Фильм с таким названием не найден!");
        return "";
    }

    // если вернулось несколько строк
    if (PQntuples(sql_result) > 1) {
        PQclear(sql_result);
        set_error(response, 400, "Найдено несколько фильмов с таким названием! Введите название более точно");
        return "";
    }

    std::string filename = PQgetvalue(sql_result, 0, 0);  // достаем значение из sql-результата 
    PQclear(sql_result);

    return "../Movies/" + filename;
}

void EMServer::handle_watch(const Request& request, Response& response) {
    std::string title = request.get_param_value("title");
    if (title.empty()) {
        set_error(response, 400, "Не передано название фильма!");
        return;
    }

    std::string playlist_filepath = get_playlist_filepath(title, response);  // получаем путь к hls плейлисту фильма
    if (playlist_filepath == "") {
        return;  // уже отправили ответ
    }

    send_hls_playlist(playlist_filepath, response);  // отправляем плейлист клиенту
}

void EMServer::handle_segment(const Request& request, httplib::Response& response) {
    // формируем путь до нужного сегмента
    std::string segment_filepath = "../Movies" + request.path.substr(0, request.path.rfind("_"));
    segment_filepath += request.path;
    
    // открываем сегмент в бинарном виде
    std::ifstream fin(segment_filepath, std::ios::binary);
    if (!fin) {
        set_error(response, 404, "Файл сегмента не найден!");
        return;
    }

    // читаем данные в строковый поток
    std::stringstream segment_stream;
    segment_stream << fin.rdbuf();
    fin.close();

    response.status = 200;
    response.set_header("Content-Type", "video/MP2T");
    response.body = segment_stream.str();
}

void EMServer::handle_get(const Request& request, httplib::Response& response) {
    std::cout << "Получен GET-запрос с путем: " << request.path << std::endl;

    // проверка, авторизован ли клиент
    if (!is_authorized(request, response)) {
        return;
    }

    PGresult* sql_result = nullptr;

    if (request.path == "/movie_list") {
        sql_result = handle_movie_list(request, response);
    }
    else if (request.path == "/watch") {
        handle_watch(request, response);
        return;
    } 
    else if (request.path.rfind(".ts") == request.path.length() - 3) {
        handle_segment(request, response);
        return;
    }
    else {
        set_error(response, 404, "Маршрут не найден!");
        return;
    }

    // проверка на ошибки при выполнении sql-запроса
    if (PQresultStatus(sql_result) != PGRES_TUPLES_OK) {
        set_error(response, 500, db.get_sql_error());
        PQclear(sql_result);
        return;
    }

    json json_res = pgresult_to_json(sql_result);  // преобразуем результат в json
    PQclear(sql_result);  // очищаем память после использования результата sql запроса
    response.set_content(json_res.dump(), "application/json");  // устанавливаем содержимое HTTP ответа в виде json
}

PGresult* EMServer::handle_reg(const json& body, Response& response) {
    // проверяем наличие всех необходимых полей
    if (!body.contains("username") || !body.contains("password")) {
        set_error(response, 400, "Ожидаются поля: username (text), password (text)!");
        return nullptr;
    }

    return safe_sql_query(response, [&] {
        // лямбда захватывает все по ссылке
        // получаем значения полей
        std::string username = body["username"].get<std::string>();
        std::string password = body["password"].get<std::string>();

        // формируем запрос
        std::string sql_query = "INSERT INTO users (username, password) VALUES (";
        sql_query += "'" + username + "', ";
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
                std::string token = generate_token();  // генерируем новый токен
                sessions[token].login = login;  // добавляем в пару к токену логин
                sessions[token].is_admin = (PQgetvalue(sql_result, 0, 0)[0] == 't');  // флаг администратора - туда же

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
    if (!body.contains("id") || !body.contains("title") || !body.contains("year") || !body.contains("filepath")) {
        set_error(response, 400, "Ожидаются поля: id (int), title (text), year (int), filepath (text)!");
        return nullptr;
    }

    return safe_sql_query(response, [&] {
        // получаем значения полей
        int id = std::stoi(body["id"].get<std::string>());
        std::string title = body["title"].get<std::string>();
        int year = std::stoi(body["year"].get<std::string>());
        std::string filepath = body["filepath"].get<std::string>();

        // формируем sql-запрос
        std::string sql_query = "INSERT INTO movies (id, title, year, filepath) VALUES (" + std::to_string(id) + ", ";
        sql_query += "'" + title + "', ";
        sql_query += std::to_string(year) + ", ";
        sql_query += "'" + filepath + "')";

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

void EMServer::handle_post(const Request& request, httplib::Response& response) {
    std::cout << "Получен POST-запрос с путем: " << request.path << std::endl;

    if (request.path != "/auth" && request.path != "/reg") {  // если не авторизация и не регистрация
        // проверка, авторизован ли клиент
        if (!is_authorized(request, response)) {
            return;
        }
    }
    
    try {
        PGresult* sql_result = nullptr;
        auto body = request.body.empty() ? json{} : json::parse(request.body);
        
        if (request.path == "/reg") {
            sql_result = handle_reg(body, response);
        }
        else if (request.path == "/auth") {
            sql_result = handle_auth(body, response);
        }
        else if (request.path == "/add_movie") {
            if (!is_admin(request, response)) {
                return;
            }
            sql_result = handle_add_movie(body, response);
        }
        else if (request.path == "/logout") {
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