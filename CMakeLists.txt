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
        include/utils/stream_container.h
        include/utils/common_util.h
        src/http/request_parser.cpp
        src/http/http_server.cpp
        include/enums/message_level.h
        src/database/connection_pool.cpp
        src/database/database.cpp
        src/model/user.cpp
        src/model/order.cpp
        src/handlers/user/user_handler.cpp
        src/handlers/user/user_query_handler.cpp
        src/handlers/user/user_create_handler.cpp
        src/handlers/user/user_update_handler.cpp
        src/handlers/user/user_delete_handler.cpp
        src/handlers/user/user_delete_handler.cpp
        src/handlers/user/user_batch_handler.cpp
        src/handlers/order/order_handler.cpp
        src/handlers/order/order_query_handler.cpp
        src/handlers/order/order_create_handler.cpp
        src/handlers/order/order_update_handler.cpp
        src/handlers/order/order_delete_handler.cpp
        src/handlers/order/order_batch_handler.cpp
        include/register/register_center.h
        src/register/register_center.cpp
        src/handlers/handler.cpp
        include/log/logger.h


)

#target_link_libraries(${PROJECT_NAME} ${Boost_LIBRARIES})
target_link_libraries(${PROJECT_NAME} PRIVATE
        Boost::headers
        Boost::json
        Boost::system
        pthread
        sqlite3)
target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_SOURCE_DIR}/include ${Boost_INCLUDE_DIRS})



