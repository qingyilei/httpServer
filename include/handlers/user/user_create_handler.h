//
// Created by huangjun on 2025/2/27.
//

#pragma once
#include <string>
#include <boost/json/value.hpp>
#include "handlers/handler.h"

class UserCreateHandler :public Handler{
public:
     std::string handle(const HttpRequest &request) override;
};