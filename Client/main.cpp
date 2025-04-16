#include "client.hpp"

#include <cpp-httplib/httplib.h>

using namespace httplib;

int main() {
    EMClient emclient ("localhost", 8080);  // создаем клиента

    emclient.enter_login_password();

    while (!emclient.authorization()) {  // пока не выполнится авторизации
        emclient.enter_login_password();  // ввод логига и пароля (либо регистрация)
    }

    return 0;
}