#include "database.hpp"
#include "server.hpp"

#include <iostream>
#include <string>
#include <cpp-httplib/httplib.h>
#include <signal.h>

using json = nlohmann::json;
using namespace httplib;

std::condition_variable cv;  // будет будить основной поток, когда надо будет остановить сервер
std::mutex cv_mutex;  // мьютекс для синхронизации доступа к переменной stop_server
bool stop_server = false;  // флаг, надо ли останавливать сервер

void sigint_handler(int sigint) {
    std::cout << "\nПолучен сигнал о завершении работы сервера.\n";
    {
        std::lock_guard<std::mutex> lg(cv_mutex);  // гарантируем, что только один поток будет изменять stop_server
        stop_server = true;
    }
    cv.notify_one();  // сообщаем основному потоку, что сервер надо остановить
}

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
    if (argc == 1) {
        std::cerr << "Не передан логин!\n";
        exit(1); 
    }

    if (argc == 2) {
        std::cerr << "Не передан пароль!\n";
        exit(1); 
    }

    std::string conn_info = "dbname=EMovieDB host=localhost user=" + std::string(argv[1]) + " password=" + argv[2];

    PGconn* conn = connect_to_db(conn_info);  // подключаемся к бд

    Server server;

    setup_routes(server, conn);  // настраиваем маршруты
    std::cout << "Маршруты сервера настроены!\n";

    signal(SIGINT, sigint_handler);  // устанавливаем обработчик сигнала ctrl+c

    // запускаем сервер в отдельном потоке
    std::thread server_thread([&server]() {
        std::cout << "Сервер запущен на http://localhost:8080\n";
        if (!server.listen("0.0.0.0", 8080)) {  // начинаем слушать запросы
            std::cerr << "Не удалось запустить сервер (возможно, порт занят)\n";
            {
                std::lock_guard<std::mutex> lg(cv_mutex);  // гарантируем, что только один поток будет изменять stop_server
                stop_server = true;
            }
            cv.notify_one();  // сообщаем основному потоку, что сервер надо остановить
        }
    });

    sleep(1);
    std::cout << "Нажмите Ctrl+C для завершения работы сервера...\n";

    // ждем сигнала ctrl+c
    {
        std::unique_lock<std::mutex> ul(cv_mutex);  // захватываем мьютекс
        cv.wait(ul, [] { return stop_server; });  // ждем пока stop_server не станет true
    }

    server.stop();  // останавливаем сервер
    server_thread.join();  // завершаем поток сервера
    std::cout << "Работа сервера остановлена.\n";

    PQfinish(conn);
    std::cout << "Соединение с базой данных завершено.\n";

    return 0;
}