#pragma once

#include "base_client.hpp"

class EMClient : public BaseClient {
private:
    std::string login;
    std::string password;
    std::string token;

    // метод регистрации нового клиента
    void registration();

    // метод ввода логина и пароля (или запрос на регистрацию)
    void enter_login_password();

    // метод ожидания авторизации клиента
    bool authorization();
    
    // метод завершения сессии
    void logout();

public:
    bool is_admin = false;  // флаг, является ли клиент администратором

    // конструктор
    EMClient(const std::string& host, int port)
        : BaseClient(host, port) {}  // инициализирует поле client

    // деструктор
    ~EMClient();

    // метод ожидания авторизации клиента
    void wait_authorization();

    // метод отображения списка фильмов
    void show_movie_list();

    // метод просмотра фильма
    void watch_movie();

    // метод выхода из профиля
    void exit_from_profile();

    // метод добавления фильма
    void add_movie();
};
