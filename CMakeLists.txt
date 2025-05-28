cmake_minimum_required(VERSION 3.10)  # минимально допустимая версия cmake
project(EMovie)  # имя проекта

set(CMAKE_CXX_STANDARD 17)  # указываем, что используется стандарт C++17

set(CMAKE_BUILD_TYPE Debug)  # отладочная информация

add_compile_options(-fdiagnostics-color=always)  # цветная диагностика компилятора

find_package(PostgreSQL REQUIRED)  # ищем установленную на системе PostgreSQL

# подключаемые директории
include_directories(
    include
    ${PostgreSQL_INCLUDE_DIRS}
)

# исполняемый файл сервера
add_executable(server
    Server/database.cpp
    Server/server.cpp
    Server/main.cpp 
)   
target_link_libraries(server ${PostgreSQL_LIBRARIES})  # подключаем необходимые библиотеки PostgreSQL

# исполняемый файл клиента
add_executable(client 
    Client/base_client.cpp
    Client/em_client.cpp 
    Client/main.cpp 
)


