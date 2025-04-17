#include "client.hpp"

using namespace httplib;
using json = nlohmann::json;

void EMClient::enter_login_password() {
    std::string answer;
    bool correct_answer = false;
    while (!correct_answer) {
        std::cout << "Вы уже зарегестрированы? (y/n): ";
        std::cin >> answer;
        
        if (answer == "y" || answer == "n") {
            correct_answer = true;
        }
    }

    //TODO: проверять что введено ровно одно значение
    if (answer == "y") {  // если уже зарегистрирован
        std::cout << "Введите логин: ";
        std::cin >> login;
        std::cout << "Введите пароль: ";
        std::cin >> password;
    }
    else {
        // регистрация
    }
}

bool EMClient::authorization() {
    json body = { {"login", login}, {"password", password} };
    Result result = client.Post("/auth", body.dump(), "application/json");  // отправляем post-запрос серверу

    if (!result || result->status != 200) {
        std::cout << "Ошибка авторизации: " << (result ? result->body : "нет ответа от сервера!") << "\n";
        return false;
    }

    // если авторизация успешна
    token = json::parse(result->body)["token"];

    headers = { {"Authorization", "Bearer " + token} };

    return true;
}

json EMClient::send_get(const std::string& route) {
    Result result = client.Get(route, headers);

    if (result->status == 200 && result) {
        return json::parse(result->body);
    }
    else if (result) {
        std::cerr << "Ошибка " << result->status << ": " << result->body << "\n";
    }
    else {
        std::cerr << "Не удалось подключиться к серверу!\n";
    }

    return {};
 }

 void EMClient::show_movie_list() {
    json movies = send_get("/show_movie_list");
    
    if (!movies.empty()) {
        std::cout << "\tСписок фильмов:\n";
        for (auto it = movies.begin(); it != movies.end(); ++it) {
            const auto& movie = *it;
            std::cout << "\t-" << movie["title"] << " (" << movie["year"] << ")";
            if (std::next(it) != movies.end()) {
                std::cout << ";\n";
            } else {
                std::cout << "\n";
            }
        }
        std::cout << "\n";
    }
 }