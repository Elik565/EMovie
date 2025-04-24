#include "em_client.hpp"
#include <iostream>

using json = nlohmann::json;

void EMClient::registration() {
    std::cout << "\n\tРегистрация:\n";

    std::cout << "Введите имя пользователя: ";
    std::cin >> login;
    std::cout << "Придумайте пароль: ";
    std::cin >> password;

    json body = { {"username", login}, {"password", password} };
    auto result = send_post("/reg", body);

    // если регистрация успешна
    if (!result.empty()) {
        std::cout << "Регистрация успешна!\n\n";
    }
    else {  // если регистрация не успешна
        login.clear();
    }
}

void EMClient::enter_login_password() {
    std::string answer;
    do {
        std::cout << "Вы уже зарегистрированы? (y/n): ";
        std::cin >> answer;
    } while (answer != "y" && answer != "n");

    if (answer == "y") {  // если уже зарегистрирован
        std::cout << "Введите логин: ";
        std::cin >> login;
        std::cout << "Введите пароль: ";
        std::cin >> password;
    } else {
        registration();
    }
}

bool EMClient::authorization() {
    if (login.empty()) return false;

    json body = { {"login", login}, {"password", password} };
    auto result = send_post("/auth", body);

    if (result.empty()) {
        return false;
    }

    // если авторизация успешна
    std::cout << "Авторизация успешна!\n\n";
    token = result["token"];  // получаем токен
    is_admin = result["is_admin"];  // проверяем, является ли клиент администратором
    headers = { {"Authorization", "Bearer " + token} };  // заполняем заголовок http запроса

    return true;
}

void EMClient::logout() {
    auto result = send_post("/logout", {});

    if (!result.empty()) {
        std::cout << result["message"] << "\n\n";
    }
}


EMClient::~EMClient() {
    std::cout << "Завершение сессии...\n";
    logout();  // завершаем сессию
}

void EMClient::wait_authorization() {
    while (!authorization()) {  // пока не выполнится авторизация
        enter_login_password();  // ввод логига и пароля (либо регистрация)
    }
}

void EMClient::show_movie_list() {
    auto movies = send_get("/movie_list");

    // если есть данные о фильмах
    if (!movies.empty()) {
        std::cout << "\tСписок фильмов:\n";
        for (const auto& movie : movies) {
            std::cout << "\t- " << movie["title"] << " (" << movie["year"] << ")\n";
        }
        std::cout << "\n";
    }
}

void EMClient::add_movie() {
    // все делаем строками, т.к. на сервере есть обработка неверных типов данных
    std::string id, title, year;

    std::cout << "Введите id: ";
    std::cin >> id;
    std::cin.ignore();
    std::cout << "Введите название: ";
    std::getline(std::cin, title);
    std::cout << "Введите год: ";
    std::cin >> year;

    // формируем тело нового фильма
    json body = { {"id", id}, {"title", title}, {"year", year} };
    auto result = send_post("/add_movie", body);

    if (!result.empty()) {
        std::cout << result["message"] << "\n";
    }
}

void EMClient::exit_from_profile() {
    logout();  // завершаем сессию
    login.clear();  // очищаем логин
    wait_authorization();  // снова ждем авторизации
}
