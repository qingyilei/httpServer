//
// Created by qing on 25-3-10.
//

#ifndef HTTP_SERVER_CONVERT_UTIL_H
#define HTTP_SERVER_CONVERT_UTIL_H

#include <string>
#include <sstream>
#include <map>
#include <charconv>

template<typename T>
inline T get_param(const std::multimap<std::string, std::string> &params,
                   const std::string &key, T default_val = T{}) {
    auto it = params.find(key);
    if (it == params.end()) return default_val;

    std::string_view sv = it->second;
    T result{};
    auto [ptr, ec] = std::from_chars(sv.data(), sv.data() + sv.size(), result);

    if (ec == std::errc()) return result;
    return default_val;
}


// string类型特化（直接返回原始字符串）
template<>
inline std::string get_param<std::string>(const std::multimap<std::string, std::string> &params,
                                          const std::string &key, std::string default_val) {
    auto it = params.find(key);
    return (it != params.end()) ? it->second : default_val;
}

// bool类型特化
template<>
inline bool get_param<bool>(const std::multimap<std::string, std::string> &params,
                            const std::string &key, bool default_val) {
    auto it = params.find(key);
    if (it == params.end()) return default_val;

    std::string value;
    std::transform(it->second.begin(), it->second.end(),
                   std::back_inserter(value), ::tolower);

    if (value == "true" || value == "1" || value == "on") return true;
    if (value == "false" || value == "0" || value == "off") return false;
    return default_val;
}

// int类型特化
template<>
inline int get_param<int>(const std::multimap<std::string, std::string> &params,
                          const std::string &key, int default_val) {
    auto it = params.find(key);
    if (it == params.end()) return default_val;

    std::string value;
    std::transform(it->second.begin(), it->second.end(),
                   std::back_inserter(value), ::tolower);
    return value.empty() ? default_val : std::stoi(value);
}
// 通用模板（数值类型）
template<typename T>
inline std::vector<T> get_all_params(const std::multimap<std::string, std::string>& params,
                             const std::string& key) {
    std::vector<T> results;
    auto range = params.equal_range(key);
    for (auto it = range.first; it != range.second; ++it) {
        T value{};
        std::from_chars(it->second.data(), it->second.data() + it->second.size(), value);
        results.push_back(value);
    }
    return results;
}

// 字符串特化
template<>
inline std::vector<std::string> get_all_params<std::string>(
    const std::multimap<std::string, std::string>& params, const std::string& key) {
    std::vector<std::string> results;
    auto range = params.equal_range(key);
    for (auto it = range.first; it != range.second; ++it) {
        results.push_back(it->second);
    }
    return results;
}

// 布尔类型特化
template<>
inline std::vector<bool> get_all_params<bool>(
    const std::multimap<std::string, std::string>& params, const std::string& key) {
    std::vector<bool> results;
    auto range = params.equal_range(key);
    for (auto it = range.first; it != range.second; ++it) {
        std::string val = it->second;
        std::transform(val.begin(), val.end(), val.begin(), ::tolower);
        if (val == "true" || val == "1" || val == "on") {
            results.push_back(true);
        } else if (val == "false" || val == "0" || val == "off") {
            results.push_back(false);
        }
    }
    return results;
}
template<typename T>
inline std::string vector_to_string(const std::vector<T> &vec) {
    if (vec.empty()) {
        return {""};
    }
    std::ostringstream oss;
    for (size_t i = 0; i < vec.size(); ++i) {
        if constexpr (std::is_same_v<T, std::string>) {
            oss << "'" << vec[i] << "'";  // 字符串类型加单引号
        } else {
            oss << vec[i];                // 数值类型直接输出
        }
        if (i != vec.size() - 1) {
            oss << ",";
        }
    }
    return oss.str();
}



#endif //HTTP_SERVER_CONVERT_UTIL_H