#include "client.hpp"

using namespace httplib;
using json = nlohmann::json;

json check_http_result(Result& result) {
    if (result) {
        if (result->status == 200) {
            return json::parse(result->body);
        } else {
            std::cerr << "Ошибка " << result->status << ": " << result->body << "\n\n";
        }
    } else {
        std::cerr << "Ошибка: нет ответа от сервера!\n\n";
    }

    return {};
}

EMClient::~EMClient() {
    std::cout << "Завершение сессии...\n";
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

void EMClient::registration() {
    std::cout << "\n\tРегистрация:\n";

    std::string username;
    std::string password;
    std::cout << "Введите имя пользователя: ";
    std::cin >> username;
    std::cout << "Придумайте пароль: ";
    std::cin >> password;

    json body = { {"username", username}, {"password", password} };
    json result = send_post("/reg", body);

    if (result.empty()) {
        return;
    }

    // если регистрация успешна
    std::cout << "Регистрация успешна!\n\n";
    login = username;
    EMClient::password = password;
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

    if (answer == "y") {  // если уже зарегистрирован
        std::cout << "Введите логин: ";
        std::cin >> login;
        std::cout << "Введите пароль: ";
        std::cin >> password;
    }
    else if (answer == "n") {
        registration();
    }
}

bool EMClient::authorization() {
    if (login.empty()) {
        return false;
    }

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
    json movies = send_get("/movie_list");

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

void EMClient::add_movie() {
    // все делаем строками, т.к. на сервере есть обработка неверных типов данных
    std::string id;
    std::string title;
    std::string year;

    std::cout << "Введите id: ";
    std::cin >> id;
    std::cout << "Введите название: ";
    std::cin.ignore();
    std::getline(std::cin, title);
    std::cout << "Введите год: ";
    std::cin >> year;

    json body = { {"id", id}, {"title", title}, {"year", year} };

    json result = send_post("/add_movie", body);

    if (!result.empty()) {
        std::cout << result["message"] << "\n";
    }
}

void EMClient::logout() {
    json result = send_post("/logout", {});

    if (!result.empty()) {
        std::cout << result["message"] << "\n\n";
    }
}