
#include "http/request_parser.h"
#include "handlers/user/user_update_handler.h"
#include "handlers/user/user_query_handler.h"
#include "handlers/user/user_delete_handler.h"
#include <boost/algorithm/string.hpp>
#include "config/server_config.h"
#include <sstream>
#include <iostream>

#include <utility>
#include <stdexcept>


std::optional<http_request> RequestParser::parse(const std::string &raw_request) {
    http_request req;
    std::istringstream iss(raw_request);  // 使用传入的 raw_request 而不是硬编码的字符串
    std::string line;

    // 解析请求行
    if (!std::getline(iss, line)) return std::nullopt;
    std::vector<std::string> parts;
    boost::split(parts, line, boost::is_any_of(" "));
    if (parts.size() < 3) return std::nullopt;  // 请求行应至少包含三个部分：方法、路径和协议版本
    req.method = parts[0];
    req.path = parts[1];

    // 解析Headers和Body
    parseParams(req);
    parsePath(req);
    if (!parseHeaders(iss, req)) return std::nullopt;
    if (!parseBody(iss, req)) return std::nullopt;

    return req;
}

bool RequestParser::parseParams(http_request &request) {
    std::multimap<std::string, std::string> params;

    size_t query_start = request.path.find('?');
    if (query_start == std::string::npos) {
        return false;
    }
    const std::string &query = request.path.substr(query_start + 1);
    size_t start = 0;
    while (start < query.size()) {
        size_t end = query.find('&', start);
        if (end == std::string::npos) end = query.size();

        std::string pair = query.substr(start, end - start);
        size_t eq_pos = pair.find('=');

        std::string key, value;
        if (eq_pos != std::string::npos) {
            key = urldecode(pair.substr(0, eq_pos));
            value = urldecode(pair.substr(eq_pos + 1));
        } else {
            key = urldecode(pair);
        }
        if (!key.empty()) {
            params.emplace(std::move(key), std::move(value));
        }

        start = end + 1;
    }
    request.params = std::move(params);
    return true;
}

bool RequestParser::parsePath(http_request &request) {
    size_t domain_start = 0;
    size_t domain_end = std::string::npos;
    size_t operation_start = std::string::npos;
    size_t operation_end = std::string::npos;
    std::string path = request.path;
    // 跳过开头的所有斜杠
    while (domain_start < path.size() && path[domain_start] == '/') {
        ++domain_start;
    }

    if (domain_start >= path.size()) {
        throw std::invalid_argument("Invalid path: no domain found");
    }

    // 查找业务领域结束位置
    domain_end = path.find('/', domain_start);
    if (domain_end == std::string::npos) {
        domain_end = path.find('?', domain_start);
    }

    if (domain_end == std::string::npos) {
        domain_end = path.size();
    }
    // 处理没有操作部分的情况
    if (domain_end == std::string::npos) {
        throw std::invalid_argument("Invalid path: no operation found");
    }

    // 跳过中间的所有斜杠
    operation_start = domain_end;
    while (operation_start < path.size() && path[operation_start] == '/') {
        ++operation_start;
    }

    // 查找操作结束位置
    operation_end = path.find('/', operation_start);
    if (operation_end == std::string::npos) {
        operation_end = path.find('?', operation_start);
    }
    if (operation_end == std::string::npos) {
        operation_end = path.size();
    }

    request.domainOperator = std::make_pair(
            path.substr(domain_start, domain_end - domain_start),
            path.substr(operation_start, operation_end - operation_start)
    );
    return true;
}


bool RequestParser::parseHeaders(std::istringstream &iss, http_request &req) {
    std::string line;
    while (std::getline(iss, line)) {
        line.erase(std::remove(line.begin(), line.end(), '\r'), line.end());
        if (line.empty()) {
            return true;  // 空行表示Header部分结束
        }
        // 假设这里解析 header，根据实际需求添加代码
        size_t colon_pos = line.find(':');
        if (colon_pos != std::string::npos) {
            std::string key = line.substr(0, colon_pos);
            std::string value = line.substr(colon_pos + 1);
            boost::algorithm::trim(key);
            boost::algorithm::trim(value);
            req.headers[key] = value;  // 使用headers属性存储键值对
        }
    }
    return false;  // 如果没有找到空行，表示Header解析失败
}

bool RequestParser::parseBody(std::istringstream &iss, http_request &req) {
    std::string body_content;
    iss.seekg(0, std::ios::end);
    size_t size = iss.tellg();
    iss.seekg(0, std::ios::beg);
    body_content.assign(std::istreambuf_iterator<char>(iss), std::istreambuf_iterator<char>());
    try {
        // 去除首尾空白字符
        boost::algorithm::trim(body_content);
        // 新增逻辑：检查 Content-Type 是否为 multipart/form-data
        auto content_type_iter = req.headers.find("Content-Type");
        if (content_type_iter != req.headers.end() &&
            content_type_iter->second.find("multipart/form-data") != std::string::npos) {
            // 提取 boundary 参数
            size_t boundary_pos = content_type_iter->second.find("boundary=");
            if (boundary_pos != std::string::npos) {
                // 调用 multipart 解析方法（需传递 ServerConfig 实例）
                std::string boundary_str = content_type_iter->second.substr(boundary_pos);
                // 去除前导空格和引号
                size_t first_non_space = boundary_str.find_first_not_of(" \t");
                boundary_str = boundary_str.substr(first_non_space);
                size_t eq_pos = boundary_str.find('=');
                if (eq_pos != std::string::npos) {
                    boundary_str = boundary_str.substr(eq_pos + 1); // 截取等号后的值
                    // 去除引号（如果存在）
                    if (!boundary_str.empty() && boundary_str.front() == '"') {
                        boundary_str = boundary_str.substr(1, boundary_str.size() - 2);
                    }
                }
                std::string boundary = boundary_str;
                req.body = body_content;
                bool multi = isMultipartComplete(body_content, boundary);
                std::cout << " complete multi part " << multi << std::endl;
                parse_multipart_form_data(req, boundary); // 假设 config 是已传入的配置
            }
            return true;
        } else {
            req.body = body_content.empty() ? boost::json::parse("{}") : boost::json::parse(body_content);
        }
    } catch (const boost::system::system_error &e) {
        // 记录具体错误信息
        std::cerr << "JSON parse error: " << e.what() << std::endl;
        return false;
    }
    return true;
}

void RequestParser::parse_multipart_form_data(http_request &req, const std::string &boundary) {
    const std::string delimiter = "--" + boundary;
    const std::string end_delimiter = delimiter + "--"; // 添加末尾双连字符

    std::cout << "Parsed boundary: " << boundary << std::endl; // 输出提取的boundary
    std::cout << "Delimiter: " << delimiter << std::endl;
    std::cout << "End delimiter: " << end_delimiter << std::endl;

    const std::string &body = req.body.as_string().c_str(); // 获取原始数据
    const ServerConfig config;
    size_t pos = 0;

    // 跳过前导边界
    pos = body.find(delimiter);
    if (pos == std::string::npos) throw std::invalid_argument("Invalid multipart data");
    pos += delimiter.length() + 2; // 跳过CRLF

    while (pos < body.size()) {
        size_t part_start = pos;
        size_t part_end = body.find(end_delimiter, pos);
        size_t delimiter_pos = body.find(delimiter, pos);
        if (part_end != std::string::npos && (part_end < delimiter_pos || delimiter_pos == std::string::npos)) {
            part_end = part_end; // 优先处理结束边界
        } else {
            part_end = delimiter_pos;
        }
        if (part_end == std::string::npos) {
            // 未找到任何边界，说明数据不完整
            Logger::getInstance().error("Incomplete multipart data");
            break;
        }
        if (part_end == std::string::npos) part_end = body.find(end_delimiter, pos);
        if (part_end == std::string::npos) break;

        // 提取单个part内容
        std::string part = body.substr(part_start, part_end - part_start - 2); // 去掉结尾CRLF
        pos = part_end + delimiter.length() + 2;

        // 解析part头
        size_t header_end = part.find("\r\n\r\n");
        if (header_end == std::string::npos) continue;

        // 解析头部信息
        std::string headers_str = part.substr(0, header_end);
        std::string content = part.substr(header_end + 4);

        std::string filename, content_type, field_name;
        std::istringstream header_iss(headers_str);
        std::string header_line;

        while (std::getline(header_iss, header_line)) {
            if (header_line.find("Content-Disposition:") != std::string::npos) {
                // 解析字段名和文件名
                size_t name_pos = header_line.find("name=\"");
                if (name_pos != std::string::npos) {
                    name_pos += 6;
                    size_t end_quote = header_line.find("\"", name_pos);
                    field_name = header_line.substr(name_pos, end_quote - name_pos);
                }

                size_t filename_pos = header_line.find("filename=\"");
                if (filename_pos != std::string::npos) {
                    filename_pos += 10;
                    size_t end_quote = header_line.find("\"", filename_pos);
                    filename = sanitize_filename(
                            header_line.substr(filename_pos, end_quote - filename_pos)
                    );
                }
            } else if (header_line.find("Content-Type:") != std::string::npos) {
                content_type = header_line.substr(13);
                boost::algorithm::trim(content_type);
            }
        }

        // 如果是文件上传
        if (!filename.empty() && !field_name.empty()) {
            // 生成安全路径
            std::string temp_path = config.get_upload_path() + "/.tmp_" + generate_uuid();
            if (temp_path.find("..") != std::string::npos) {
                throw std::runtime_error("Invalid file path");
            }
            try {
                // 创建内存映射
                auto mapping = FileUploader::map_file(temp_path,filename,  content.size());

                // 直接拷贝内容到映射内存
                memcpy(mapping.addr, content.data(), content.size());
                msync(mapping.addr, content.size(), MS_SYNC);

                // 填充上传文件信息
                http_request::UploadedFile uploaded_file{
                        .filename = filename,
                        .content_type = content_type,
                        .size = content.size(),
                        .mapping = mapping
                };

                req.uploaded_files.emplace(field_name, std::move(uploaded_file));
                FileUploader::unmap_file(mapping);
            } catch (const std::exception &e) {
                throw std::runtime_error("File upload failed: " + std::string(e.what()));
            }
        } else {
            // 处理普通表单字段
            req.params.emplace(field_name, content);
        }
    }
}

// 解析multipart数据，检测文件上传部分
// 当识别到文件字段时：
// 1. 创建临时文件路径
// 2. 使用FileUploader::map_file映射内存
// 3. 将文件内容直接写入映射内存
// 4. 填充req.uploaded_files
//}
