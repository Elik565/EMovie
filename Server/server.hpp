#pragma once

#include <cpp-httplib/httplib.h>
#include "database.hpp"

void handle_get(httplib::Server& server, PGconn* conn, const std::string& route, const std::string& query);

void setup_routes(httplib::Server& server, PGconn* conn);