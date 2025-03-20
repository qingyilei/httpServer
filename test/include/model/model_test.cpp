//
// Created by qing on 25-3-16.
//
// test_model.cpp
#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "../../../include/model/model.h"


using namespace testing;

// Mock派生类定义
class MockModel : public Model<MockModel> {
public:
    using Model::fields_; // 暴露protected成员

    std::vector<std::pair<MessageLevel, std::string>> verify() override {
        return {}; // 测试时动态设置
    }

    static void register_fields() {

    }

    json::object to_json() const override {
        return {}; // 测试时不需要实际实现
    }
};

// 测试类
class ModelTest : public ::testing::Test {
protected:
    void SetUp() override {
        MockModel::register_fields();
        // 清理单例实例（如果需要）
        // 注意：单例的实例在测试中可能需要手动重置，但此处假设测试用例间无依赖
    }
};


// Mock Logger
class MockLogger : public Logger {
public:
    MOCK_METHOD(void, error, (const std::string&), ());
};
TEST(ModelTest, SaveInsertNewRecord) {
    MockModel model;
    model.fields_["name"] = "Test";

    // 执行保存
   model.save();

    // 验证ID被设置
    EXPECT_GT(model.get<int>("id"), 0);
}
//
//TEST(ModelTest, ProcessVerifyWithErrors) {
//    MockModel model;
//    std::vector<std::pair<MessageLevel, std::string>> errors = {
//        {MessageLevel::ERROR, "Field required"},
//        {MessageLevel::WARNING, "Optional field"}
//    };
//
//    // 注入错误
//    model.verify() = [&errors]() { return errors; };
//
//    // 验证异常和日志
//    MockLogger mock_logger;
//    EXPECT_CALL(mock_logger, error(HasSubstr("Field required")));
//
//    EXPECT_THROW(model.process_verify(), std::runtime_error);
//}
//
//TEST(ModelTest, GetSetFieldValues) {
//    MockModel model;
//
//    // 测试类型安全
//    model.set("age", 25);
//    EXPECT_EQ(model.get<int>("age"), 25);
//
//    model.set("price", 19.99);
//    EXPECT_DOUBLE_EQ(model.get<double>("price"), 19.99);
//
//    // 测试异常
//    EXPECT_THROW(model.get<int>("invalid_field"), std::invalid_argument);
//}
//
//TEST(ModelTest, RemoveOperation) {
//    MockModel model;
//    model.set("id", 100);
//
//    SqlOperator<MockModel> mock_sql;
//    EXPECT_CALL(mock_sql, where("id", 100))
//        .WillOnce(Return(ByMove(QueryBuilder<MockModel>())));
//    EXPECT_CALL(mock_sql, remove())
//        .WillOnce(Return(ByMove(QueryBuilder<MockModel>())));
//
//    model.remove();
//    // 验证WHERE条件构建（需扩展QueryBuilder的mock）
//}
