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

    std::string handle(const http_request &request) override {
        return {""};
    }

};

