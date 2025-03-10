cmake_minimum_required(VERSION 3.20)
project(httpServer)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_OSX_ARCHITECTURES arm64)
set(Boost_NO_WARN_NEW_VERSIONS 1)
cmake_policy(SET CMP0167 NEW)

# 禁用Boost自己的CMake模块
find_package(Boost REQUIRED COMPONENTS json system)

# 验证是否找到Boost
if (Boost_FOUND)
    message(STATUS "Boost found: ${Boost_VERSION}")
else ()
    message(FATAL_ERROR "Boost not found")
endif ()


add_executable(${PROJECT_NAME} src/main.cpp
        src/http/request_parser.cpp
        src/http/http_server.cpp
        src/database/connection_pool.cpp
        src/database/database.cpp
        src/model/user_traits.cpp
        src/model/user.cpp
        src/handlers/users/user_handler.cpp
        src/handlers/users/user_query_handler.cpp
        src/handlers/users/user_create_handler.cpp
        src/handlers/users/user_update_handler.cpp
        src/handlers/users/user_delete_handler.cpp
        include/stream/stream_container.h
        include/utils/convert_util.h

)

#target_link_libraries(${PROJECT_NAME} ${Boost_LIBRARIES})
target_link_libraries(${PROJECT_NAME} PRIVATE
        Boost::headers
        Boost::json
        Boost::system
        pthread
        sqlite3)
target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_SOURCE_DIR}/include ${Boost_INCLUDE_DIRS})



