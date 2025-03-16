//
// Created by huangjun on 2025/2/27.
//

#pragma once
#include <string>
#include <boost/json/value.hpp>
#include "handlers/handler.h"

class UserBatchHandler :public Handler{
public:
     std::string handle(const http_request &request) override;
};