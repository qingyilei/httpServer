//
// Created by qing on 25-3-17.
//

#ifndef HTTPSERVER_SERVER_CONFIG_H
#define HTTPSERVER_SERVER_CONFIG_H
#include <string>
#include <filesystem>
struct ServerConfig {
    size_t max_file_size = 1024 * 1024 * 100;
    std::string upload_dir = "uploads"; // 改为相对路径

    // 获取完整上传路径
    std::string get_upload_path() const {
        namespace fs = std::filesystem;
        fs::path project_root = fs::current_path(); // 获取项目运行目录
        fs::path upload_path = project_root / upload_dir;

        // 自动创建目录
        if (!fs::exists(upload_path)) {
            fs::create_directories(upload_path);
        }
        return upload_path.string();
    }
};

#endif //HTTPSERVER_SERVER_CONFIG_H
