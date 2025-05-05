#pragma once

#include <nlohmann/json.hpp>
#include <cpp-httplib/httplib.h>
#include "database.hpp"

// функция преобразования результата sql в json
nlohmann::json pgresult_to_json(PGresult* res);

// функция генерации токена
std::string generate_token();

// функция установки сообщения об ошибке
void set_error(httplib::Response& response, const int status, const std::string& message);

// функция получения токена из запроса
std::string get_token_from_request(const httplib::Request& request, httplib::Response& response);

// функция обработки исключений при формировании sql-запроса
PGresult* safe_sql_query(httplib::Response& response, std::function<PGresult*()> func);

// функция отправки hls плейлиста
void send_hls_playlist(const std::string& filepath, httplib::Response& response);

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

    // метод проверки авторизации пользователя
    bool is_authorized(const httplib::Request& request, httplib::Response& response);

    // метод получения логина из запроса
    std::string get_login_from_request(const httplib::Request& request);

    // метод проверки, является ли клиент администратором
    bool is_admin(const httplib::Request& request, httplib::Response& response);

    // метод обработки запроса отображения списка фильмов
    PGresult* handle_movie_list(const httplib::Request& request, httplib::Response& response);

    // метод получения пути к hls плейлисту фильма
    std::string get_playlist_filepath(const std::string& title, httplib::Response& response);

    // метод обработки запроса просмтора фильма
    void handle_watch(const httplib::Request& request, httplib::Response& response);

    // метод обработки сегмента .ts
    void handle_segment(const httplib::Request& request, httplib::Response& response);

    // метод обработки GET-запроса
    void handle_get(const httplib::Request& request, httplib::Response& response);

    // метод обработки запроса регистрации нового клиента
    PGresult* handle_reg(const nlohmann::json& body, httplib::Response& response);

    // метод обработки запроса авторизации клиента
    PGresult* handle_auth(const nlohmann::json& body, httplib::Response& response);

    // метод обработки запроса добавления фильма
    PGresult* handle_add_movie(const nlohmann::json& body, httplib::Response& response);

    // метод обработки запроса завершения сессии клиента
    void handle_logout(const httplib::Request& request, httplib::Response& response);

    // метод обработки POST-запроса
    void handle_post(const httplib::Request& request, httplib::Response& response);

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

