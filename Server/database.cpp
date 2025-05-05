#include "database.hpp"
#include <iostream>

EMDatabase::EMDatabase(const std::string& conn_info) {
    conn = PQconnectdb(conn_info.c_str());  // пытаемся подключиться к бд

    if (PQstatus(conn) != CONNECTION_OK) {
        std::cerr << "Не удалось подключиться к базе данных: " << PQerrorMessage(conn);
        PQfinish(conn);
        exit(1);
    }
}

EMDatabase::~EMDatabase() {
    PQfinish(conn);
    std::cout << "Подключение к базе данных закрыто." << std::endl;
}

PGresult* EMDatabase::execute_query(const std::string& sql_query) const {
    return PQexec(conn, sql_query.c_str());
} 

std::string EMDatabase::get_sql_error() const {
    return PQerrorMessage(conn);
}
