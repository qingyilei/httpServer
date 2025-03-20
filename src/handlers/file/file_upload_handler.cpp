#include "handlers/file/file_upload_handler.h"
#include <string>
#include "model/order.h"
#include "config/server_config.h"
#include <filesystem>
namespace fs = std::filesystem;  // 新增：定义 fs 别名
std::string FileUploadHandler::handle(const http_request &req) {


    if (req.method != "POST") {
        return generate_error_response(405, "Method not allowed");
    }
    auto it = req.uploaded_files.find("file");
    if (it == req.uploaded_files.end()) {
        return generate_error_response(400, "No file uploaded");

    }
    const auto &file = it->second;
    try {
        // 处理映射文件（直接访问file.mapping.addr）
        // 示例：保存到持久化存储
        // 生成临时路径（可选，若需保留中间步骤）
        std::string temp_path = "/uploads/" + file.filename;
        if (temp_path.find("../") != std::string::npos) {
            return generate_error_response(500, "Invalid file path");
        }

        ServerConfig cfg;
        const std::string upload_base = cfg.get_upload_path();

        std::string safe_name = sanitize_filename(file.filename);
        std::string dest_path = (fs::path(upload_base) / safe_name).string();

        if (rename(file.mapping.path.c_str(), dest_path.c_str()) != 0) {
            throw std::runtime_error("File save failed");
        }
        return generate_resp<std::string>("File uploaded successfully");
    } catch (const std::exception &e) {

        return generate_error_response(500, e.what());

    }
}