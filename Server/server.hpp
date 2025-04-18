#pragma once

#include <nlohmann/json.hpp>
#include <cpp-httplib/httplib.h>
#include "database.hpp"

// функция преобразования результата sql в json
nlohmann::json pgresult_to_json(PGresult* res);

// функция генерации токена
std::string generate_token();


// структура информации о сессии
struct SessionInfo {
    std::string login;
    bool is_admin;  // флаг, является ли клиент администратором
};

class EMServer {
private:
    httplib::Server server;
    EMDatabase db;
    std::unordered_map<std::string, SessionInfo> sessions;  // хэш-таблица для быстрого доступа по ключу (токену)

    // метод установки сообщения об ошибке
    void set_error(httplib::Response& response, const int status, const std::string& message);

    // метод настройки маршрутов сервера
    void setup_routes();

    // функция получения токена из запроса
    std::string get_token_from_request(const httplib::Request& request, httplib::Response& response);

    // метод проверки авторизации пользователя
    bool is_authorized(const httplib::Request& request, httplib::Response& response);

    // метод проверки, является ли клиент администратором
    bool is_admin(const httplib::Request& request, httplib::Response& response);

    // метод обработки GET-запроса
    void handle_get(const std::string& route, const std::string& sql_query);

    // метод обработки запроса авторизации клиента
    PGresult* handle_auth(const nlohmann::json& body, httplib::Response& response);

    // метод обработки запроса добавления фильма
    PGresult* handle_add_movie(const nlohmann::json& body, httplib::Response& response);

    // метод обработки запроса завершения сессии клиента
    void handle_logout(const httplib::Request& request, httplib::Response& response);

    // метод обработки POST-запроса
    void handle_post(const std::string& route);

public:
    // конструктор
    EMServer(const std::string& conn_info);

    // деструктор
    ~EMServer();

    // метод запуска сервера
    void start();

    // метод остановки сервера
    void stop();
};

