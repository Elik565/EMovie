#include "em_client.hpp"
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

    emclient_ptr->wait_authorization();  // ждем авторизации клиента

    std::signal(SIGINT, sigint_handler);  // обработчки Ctrl+C

    std::string answer;
    while(answer != "exit") {
        std::cout << "\tAction menu:\n";
        std::cout << "1 - Show movie list;\n";
        std::cout << "2 - Start watching a movie;\n";
        std::cout << "3 - Re-login;\n";

        // действия для администратора
        if (emclient_ptr->is_admin) {
            std::cout << "4 -  Add a movie;\n";
        }

        std::cout << "exit - Quit\n\n";

        bool correct_answer = false;
        while (!correct_answer) {
            std::cout << "Ввод: ";
            std::cin >> answer;

            correct_answer = true;

            if (answer == "1") {
                emclient_ptr->show_movie_list();
            }
            else if (answer == "2") {
                emclient_ptr->watch_movie();
            }
            else if (answer == "3") {
                emclient_ptr->exit_from_profile();
            }
            else if (answer == "4" && emclient_ptr->is_admin) {
                emclient_ptr->add_movie();
            }
            else if (answer == "exit") {
                emclient_ptr.reset();  // вызываем деструктор
                break;
            }
            else {
                correct_answer = false;
            }
        }
    }

    return 0;
}