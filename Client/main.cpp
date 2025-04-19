#include "client.hpp"

#include <cpp-httplib/httplib.h>

using namespace httplib;

std::unique_ptr<EMClient> emclient_ptr;  // глобальный указатель на клиента

void sigint_handler(int sigint) {
    std::cout << "\nПолучен сигнал о завершении работы клиента.\n";
    if (emclient_ptr) {
        emclient_ptr.reset();  // вызываем деструктор
        std::cout << "Программа клиента завершена.\n";
        exit(0);
    }
}

int main() {
    emclient_ptr = std::make_unique<EMClient>("localhost", 8080);  // создаем указатель на клиента

    while (!emclient_ptr->authorization()) {  // пока не выполнится авторизация
        emclient_ptr->enter_login_password();  // ввод логига и пароля (либо регистрация)
    }

    std::signal(SIGINT, sigint_handler);
    
    std::string answer;
    while(true) {
        std::cout << "\tМеню действий:\n";
        std::cout << "1 - Показать список фильмов;\n";
        std::cout << "2 - Выйти из профиля;\n";

        // действия для администратора
        if (emclient_ptr->is_admin) {
            std::cout << "3 - Добавить фильм;\n";
        }

        std::cout << "exit - Выйти\n\n";

        std::cout << "Ввод: ";
        std::cin >> answer;

        if (answer == "1") {
            emclient_ptr->show_movie_list();
        }
        else if (answer == "3" && emclient_ptr->is_admin) {
            emclient_ptr->add_movie();
        }

        if (answer == "exit") {
            break;
        }
    }

    return 0;
}