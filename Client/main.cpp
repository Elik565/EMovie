#include "client.hpp"

#include <cpp-httplib/httplib.h>

using namespace httplib;

int main() {
    EMClient emclient ("localhost", 8080);  // создаем клиента

    emclient.enter_login_password();

    while (!emclient.authorization()) {  // пока не выполнится авторизация
        emclient.enter_login_password();  // ввод логига и пароля (либо регистрация)
    }

    std::string answer;
    while(true) {
        std::cout << "\tМеню действий:\n";
        std::cout << "1 - Показать список фильмов;\n";
        std::cout << "2 - Выйти из профиля;\n";

        // действия для администратора
        if (emclient.is_admin) {
            std::cout << "3 - Добавить фильм;\n";
        }

        std::cout << "exit - Выйти\n\n";

        std::cout << "Ввод: ";
        std::cin >> answer;

        if (answer == "1") {
            emclient.show_movie_list();
        }
        else if (answer == "3" && emclient.is_admin) {
            emclient.add_movie();
        }

        if (answer == "exit") {
            break;
        }
    }

    return 0;
}