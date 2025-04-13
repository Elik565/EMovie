#include "database.hpp"
#include <iostream>

using json = nlohmann::json;

PGconn* connect_to_db(const std::string& conn_info) {
    PGconn* conn = PQconnectdb(conn_info.c_str());

    if (PQstatus(conn) != CONNECTION_OK) {
        std::cerr << "Не удалось подключиться к базе данных: " << PQerrorMessage(conn);
        PQfinish(conn);
        exit(1);
    }

    return conn;
}

json pgresult_to_json(PGresult* res) {
    json json_res = json::array();

    int nrows = PQntuples(res);  // кол-во строк
    int ncols = PQnfields(res);  // кол-во столбцов

    for (int i = 0; i < nrows; i++) {
        json row;
        for (int j = 0; j < ncols; j++) {
            std::string col_name = PQfname(res, j);  // получаем название столбца
            row[col_name] = PQgetvalue(res, i, j);  // получаем значение
        }
        json_res.push_back(row);  // добавляем к результату
    }

    return json_res;
}
