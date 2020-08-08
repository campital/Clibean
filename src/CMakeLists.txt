cmake_minimum_required(VERSION 3.15)

project(Clibean VERSION 0.0)

set(CMAKE_CXX_STANDARD 17)

set(SRC main.cpp
        http.cpp
        netUtil.cpp
        session.cpp
        ui/BasicUI.cpp)

add_executable(clibean ${SRC})

find_package(OpenSSL REQUIRED)
target_link_libraries(clibean OpenSSL::SSL)