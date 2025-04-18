#include "client.hpp"

using namespace httplib;
using json = nlohmann::json;

json check_http_result(Result& result) {
    if (result->status == 200 && result) {
        return json::parse(result->body);
    }
    else if (result) {
        std::cerr << "Ошибка " << result->status << ": " << result->body << "\n";
    }
    else {
        std::cerr << "Нет ответа от сервера!\n";
    }

    return {};
}

EMClient::~EMClient() {
    logout();  // завершаем сессию
}

json EMClient::send_get(const std::string& route) {
    Result result = client.Get(route, headers);  // отправляем GET-запрос

    return check_http_result(result);  // проверка ответа от сервера
}

json EMClient::send_post(const std::string& route, const json& body) {
    Result result;

    if (body.empty()) {
        result = client.Post(route, headers);
    }
    else {
        result = client.Post(route, headers, body.dump(), "application/json");
    }

    return check_http_result(result);  // проверка ответа от сервера
}

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
    json result = send_post("/auth", body);

    if (result.empty()) {
        return false;
    }

    // если авторизация успешна
    std::cout << "Авторизация успешна!\n\n";
    token = result["token"].get<std::string>();  // получаем токен
    is_admin = result["is_admin"].get<bool>();  // проверяем, является ли клиент администратором
    headers = { {"Authorization", "Bearer " + token} };  // заполняем заголовок http запроса

    return true;
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

void EMClient::logout() {
    json result = send_post("/logout", {});

    if (!result.empty()) {
        std::cout << result["message"] << "\n";
    }
}