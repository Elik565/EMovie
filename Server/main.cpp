#include "database.hpp"
#include "server.hpp"
#include <cpp-httplib/httplib.h>
#include <mutex>

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

    httplib::Server server;

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