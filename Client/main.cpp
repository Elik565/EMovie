#include "client.hpp"

#include <cpp-httplib/httplib.h>

using namespace httplib;

int main() {
    Client client("localhost", 8080);
    std::string login, password;
    
    enter_login_password(login, password);

    while (!authentication(client, login, password)) {
        enter_login_password(login, password);
    }


    
    return 0;
}