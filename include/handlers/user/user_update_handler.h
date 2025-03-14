//
// Created by huangjun on 2025/2/27.
//

#pragma once
#include <string>
#include "handlers/handler.h"

class UserUpdateHandler :public Handler{
public:
    std::string handle(const HttpRequest &request) override;
};