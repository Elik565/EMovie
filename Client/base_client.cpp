#include "base_client.hpp"

using json = nlohmann::json;
using namespace httplib;

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


json BaseClient::send_get(const std::string& route) {
    auto result = client.Get(route, headers);  // отправляем GET-запрос

    return check_http_result(result);  // проверка ответа от сервера
}

json BaseClient::send_post(const std::string& route, const json& body) {
    auto result = body.empty()
        ? client.Post(route, headers)
        : client.Post(route, headers, body.dump(), "application/json");

    return check_http_result(result);  // проверка ответа от сервера
}
