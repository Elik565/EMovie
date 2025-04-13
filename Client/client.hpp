#pragma once

#include <cpp-httplib/httplib.h>

void enter_login_password(std::string& login, std::string& password);

bool authentication(httplib::Client& client, const std::string& login, const std::string& password);