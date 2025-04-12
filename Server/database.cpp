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

PGresult* execute_query(PGconn* conn, const std::string& query) {
    PGresult* res = PQexec(conn, query.c_str());

    if (PQresultStatus(res) != PGRES_TUPLES_OK) {
        std::cerr << "Ошибка выполнения запроса: " << PQerrorMessage(conn);
        PQclear(res);
        return nullptr;
    }

    return res;
}

void close_connection(PGconn* conn, PGresult* res) {
    PQclear(res);
    PQfinish(conn);
}