#include "handlers/handler.h"

class FileUploadHandler :public Handler{
private:
    std::string sanitize_filename(const std::string& name) {
        std::filesystem::path p(name);
        return p.filename().string(); // 剥离路径信息
    }
public:
    std::string handle(const http_request &request)  override;
};