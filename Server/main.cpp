#include "database.hpp"
#include "server.hpp"
#include <cpp-httplib/httplib.h>

std::unique_ptr<EMServer> emserver_ptr;  // глобальный указатель на сервер

void sigint_handler(int sigint) {
    std::cout << "\nПолучен сигнал о завершении работы сервера.\n";
    if (emserver_ptr) {
        emserver_ptr->stop();
        std::cout << "Работа сервера завершена.\n";
    }
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

    signal(SIGINT, sigint_handler);  // устанавливаем обработчик сигнала ctrl+c

    emserver_ptr = std::make_unique<EMServer>(conn_info);  // создаем указатель

    emserver_ptr->start();

    return 0;
}