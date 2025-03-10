//
// Created by huangjun on 2025/2/27.
//

#include <unordered_map>
#include <functional>
#include <string>
#include "handlers/handler.h"

class UserHandler : public Handler {

public:
    UserHandler();
    ~UserHandler() override;

    std::string handle(const HttpRequest &request) override;


private:
    std::unordered_map<std::string, std::shared_ptr<Handler>> operators_;
};

