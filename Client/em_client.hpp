#pragma once

#include "base_client.hpp"

class EMClient : public BaseClient {
private:
    std::string login;
    std::string password;
    std::string token;
    bool is_admin = false;

    void registration();
    bool authorization();
    void logout();
    void enter_login_password();

public:
    EMClient(const std::string& host, int port = 80)
        : BaseClient(host, port) {}

    ~EMClient();

    void wait_authorization();
    void show_movie_list();
    void add_movie();
    void exit_from_profile();
};
