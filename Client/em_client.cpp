#include "em_client.hpp"
#include <iostream>

using json = nlohmann::json;

EMClient::~EMClient() {
    std::cout << "Завершение сессии...\n";
    logout();
}

void EMClient::wait_authorization() {
    while (!authorization()) {
        enter_login_password();
    }
}

void EMClient::registration() {
    std::cout << "\n\tРегистрация:\n";

    std::cout << "Введите имя пользователя: ";
    std::cin >> login;
    std::cout << "Придумайте пароль: ";
    std::cin >> password;

    json body = { {"username", login}, {"password", password} };
    auto result = send_post("/reg", body);

    if (!result.empty()) {
        std::cout << "Регистрация успешна!\n\n";
    }
}

void EMClient::enter_login_password() {
    std::string answer;
    do {
        std::cout << "Вы уже зарегистрированы? (y/n): ";
        std::cin >> answer;
    } while (answer != "y" && answer != "n");

    if (answer == "y") {
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

    if (result.empty()) return false;

    std::cout << "Авторизация успешна!\n\n";
    token = result["token"];
    is_admin = result["is_admin"];
    headers = { {"Authorization", "Bearer " + token} };

    return true;
}

void EMClient::logout() {
    auto result = send_post("/logout");

    if (!result.empty()) {
        std::cout << result["message"] << "\n\n";
    }
}

void EMClient::show_movie_list() {
    auto movies = send_get("/movie_list");

    if (!movies.empty()) {
        std::cout << "\tСписок фильмов:\n";
        for (const auto& movie : movies) {
            std::cout << "\t- " << movie["title"] << " (" << movie["year"] << ")\n";
        }
        std::cout << "\n";
    }
}

void EMClient::add_movie() {
    std::string id, title, year;

    std::cout << "Введите id: ";
    std::cin >> id;
    std::cin.ignore();
    std::cout << "Введите название: ";
    std::getline(std::cin, title);
    std::cout << "Введите год: ";
    std::cin >> year;

    json body = { {"id", id}, {"title", title}, {"year", year} };
    auto result = send_post("/add_movie", body);

    if (!result.empty()) {
        std::cout << result["message"] << "\n";
    }
}

void EMClient::exit_from_profile() {
    logout();
    login.clear();
    wait_authorization();
}
