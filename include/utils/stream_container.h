//
// Created by qing on 25-3-9.
//

#ifndef HTTP_SERVER_STREAM_CONTAINER_H
#define HTTP_SERVER_STREAM_CONTAINER_H
#include <ranges>
#include <algorithm>
#include <iostream>
#include <vector>
#include <string>
#include <functional>
#include <iterator>
#include <utility>
#include <concepts>
#include <type_traits>
// 包装器实现链式操作
template<typename Container>
class Stream {
public:
    explicit Stream(Container&& c) : container(std::forward<Container>(c)) {}

    template<typename Predicate>
    auto filter(Predicate&& pred) {
        return Stream(container | std::views::filter(std::forward<Predicate>(pred)));
    }

    template<typename Mapper>
    auto map(Mapper&& mapper) {
        return Stream(container | std::views::transform(std::forward<Mapper>(mapper)));
    }

    void forEach(auto&& consumer) {
        for (auto&& elem : container) {
            consumer(std::forward<decltype(elem)>(elem));
        }
    }

private:
    Container container;
};
//
//// 使用示例
//user | as_stream()  // 自定义包装函数
//    .filter([](auto&& u) { return u.age < 30; })
//    .map([](auto&& u) { return u.name; })
//    .forEach([](auto&& name) { std::cout << name << "\n"; });
//#include <ranges>  // C++20 ranges库
//#include <vector>
//#include <algorithm>
//
//// 示例数据结构
//struct User {
//    int id;
//    std::string name;
//    int age;
//};
//
//int main() {
//    std::vector<User> user{
//            {1, "Alice", 25},
//            {2, "Bob", 30},
//            {3, "Charlie", 20}
//    };
//
//    // 流式处理管道
//    auto processed = user
//                     | std::views::filter([](const User& u) { // 过滤条件
//        return u.age > 20; // 保留年龄>20的用户
//    })
//                     | std::views::transform([](const User& u) { // 转换操作
//        return std::format("{}:{}", u.name, u.age); // 格式化输出
//    });
//
//    // 结果输出
//    for (const auto& str : processed) {
//        std::cout << str << "\n";
//    }
//}
#endif //HTTP_SERVER_STREAM_CONTAINER_H
