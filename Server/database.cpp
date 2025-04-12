#include "database.hpp"
#include <iostream>

PGconn* connect_to_db(const std::string& conn_info) {
    PGconn* conn = PQconnectdb(conn_info.c_str());

    if (PQstatus(conn) != CONNECTION_OK) {
        std::cerr << "Не удалось подключиться к базе данных: " << PQerrorMessage(conn);
        PQfinish(conn);
        return nullptr;
    }

    return conn;
}