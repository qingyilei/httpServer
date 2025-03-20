cmake_minimum_required(VERSION 3.20)
project(httpServer)


set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_OSX_ARCHITECTURES arm64)
set(Boost_NO_WARN_NEW_VERSIONS 1)
cmake_policy(SET CMP0167 NEW)
# 使用变量引入


# 禁用Boost自己的CMake模块
find_package(Boost REQUIRED COMPONENTS json system)
# 查找 Google Test
find_package(GTest REQUIRED)


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
        src/model/order_user.cpp
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
        src/handlers/order/order_user_query_handler.cpp
        src/handlers/file/file_upload_handler.cpp
        include/register/register_center.h
        src/register/register_center.cpp
        src/handlers/handler.cpp
        include/http/file_upload.h
        include/config/server_config.h
)




#target_link_libraries(${PROJECT_NAME} ${Boost_LIBRARIES})
target_link_libraries(${PROJECT_NAME} PRIVATE
        Boost::headers
        Boost::json
        Boost::system
        pthread
        sqlite3
#        ${UUID_LIBRARY}
)

target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_SOURCE_DIR}/include ${Boost_INCLUDE_DIRS})

# 添加测试目标
add_executable(httpServer_test
        test/include/sql/sql_operator_test.cpp
        test/include/model/model_test.cpp

        # 其他测试文件
)
# 链接 Google Test 库到测试目标
target_link_libraries(httpServer_test
        GTest::GTest
        GTest::Main      # 包含主函数的测试入口
        # 其他依赖库（如你的 SqlOperator 实现）
        Boost::headers
        Boost::json
        Boost::system
        pthread
        sqlite3
)

target_include_directories(httpServer_test PRIVATE ${CMAKE_SOURCE_DIR}/include ${Boost_INCLUDE_DIRS})


enable_testing()

add_test(NAME httpServer_test COMMAND sql_operator_test)

