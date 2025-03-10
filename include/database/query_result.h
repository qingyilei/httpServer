//
// Created by qing on 2025/3/5.
//
#include <boost/json.hpp>

#ifndef QUERY_RESULT_H
#define QUERY_RESULT_H
namespace json = boost::json;

template<typename Model>
class QueryResult {
public:
    struct Pagination {
        int current_page;
        int page_size;
        int total_items;
        int total_pages;
    };

    // 添加分页元数据
    QueryResult &with_metadata(Pagination meta) {
        pagination_ = meta;
        return *this;
    }

    QueryResult(std::vector<Model> data, Pagination pagination)
            : data_(std::move(data)), pagination_(pagination) {}

    QueryResult(std::vector<Model> data)
            : data_(std::move(data)){}

    // 转换为JSON对象
    [[nodiscard]] boost::json::object to_json() const {
        return {
                {"data",       serialize_data()},
                {"pagination", serialize_pagination()}
        };
    }

private:
    Pagination pagination_;
    std::vector<Model> data_;

    boost::json::array serialize_data() const {
        boost::json::array arr;
        for (const auto &item: data_) {
            arr.push_back(item.to_json());
        }
        return arr;
    }

    boost::json::object serialize_pagination() const {
        return {
                {"current_page", pagination_.current_page},
                {"page_size",    pagination_.page_size},
                {"total_items",  pagination_.total_items},
                {"total_pages",  pagination_.total_pages}
        };
    }

};

#endif //QUERY_RESULT_H
