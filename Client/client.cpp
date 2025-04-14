#include "client.hpp"
#include <nlohmann/json.hpp>

using namespace httplib;
using json = nlohmann::json;

void enter_login_password(std::string& login, std::string& password) {
    std::string answer;
    bool correct_answer = false;
    while (!correct_answer) {
        std::cout << "Вы уже зарегестрированы? (y/n): ";
        std::cin >> answer;
        
        if (answer == "y" || answer == "n") {
            correct_answer = true;
        }
    }

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

bool authentication(Client& client, const std::string& login, const std::string& password) {
    json body = { {"login", login}, {"password", password} };
    Result result = client.Post("/auth", body.dump(), "application/json");

    if (!result || result->status != 200) {
        std::cout << "Ошибка авторизации: " << (result ? result->body : "Нет ответа от сервера") << "\n";
        return false;
    }

    return true;
}