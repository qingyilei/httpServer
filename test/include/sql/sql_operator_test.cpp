//
// Created by qing on 25-3-16.
//
#include "gtest/gtest.h"
#include "../../../include/sql/sql_operator.h"
#include <iostream>


// 定义一个测试用的 Model 类
struct TestModel {
};


// 测试类
class SqlOperatorTest : public ::testing::Test {
protected:
    void SetUp() override {
        ModelTraits<TestModel>::instance().set_primary_key("id");
        ModelTraits<TestModel>::instance().set_table_name("test_table");
        // 清理单例实例（如果需要）
        // 注意：单例的实例在测试中可能需要手动重置，但此处假设测试用例间无依赖
    }
};

// 测试单例模式
TEST_F(SqlOperatorTest, TestSingleton) {
    auto &op1 = SqlOperator<TestModel>::instance();
    auto &op2 = SqlOperator<TestModel>::instance();
    EXPECT_EQ(&op1, &op2);  // 检查是否为同一实例
}

// 测试 update() 方法
TEST_F(SqlOperatorTest, TestUpdate) {
    auto table = SqlOperator<TestModel>::instance().update();
// 验证 SQL 模板和表名（假设 SqlTable 提供了访问方法）
// 或使用 Mock 捕获构造函数参数
// 此处假设 SqlTable 有公开的 get_sql() 和 get_table_name()
    EXPECT_EQ(table->operator_sql(), "UPDATE %t SET %f WHERE (%w) ");
    EXPECT_EQ(table->sql_table(), "test_table");
}

// 测试 select() 方法
TEST_F(SqlOperatorTest, TestSelect) {
    auto table = SqlOperator<TestModel>::instance().select();
// 验证 SQL 模板和表名
    EXPECT_EQ(table->operator_sql(), "SELECT %f FROM %t WHERE %w ");

}

// 测试 remove() 方法
TEST_F(SqlOperatorTest, TestRemove) {
    auto table = SqlOperator<TestModel>::instance().remove();
// 验证 SQL 模板和表名
    EXPECT_EQ(table->operator_sql(), "DELETE FROM %t WHERE %w ");
}

// 测试 insert() 方法
TEST_F(SqlOperatorTest, TestInsert) {
    auto table = SqlOperator<TestModel>::instance().insert();
    EXPECT_EQ(table->operator_sql(), "INSERT INTO %t (%f) values (%v)");
}

// 测试析构函数输出（需重定向 cout）
TEST_F(SqlOperatorTest, TestDestructor) {
    testing::internal::CaptureStdout();
    {
        SqlOperator<TestModel> op = SqlOperator<TestModel>::instance();  // 直接构造对象（绕过单例）
        op.~SqlOperator();          // 显式调用析构函数
    }
    std::string output = testing::internal::GetCapturedStdout();
    EXPECT_EQ(output, "SqlOperator destroy\n"
                      "SqlOperator destroy\n");
}
