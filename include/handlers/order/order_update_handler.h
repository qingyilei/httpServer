//
// Created by huangjun on 2025/2/27.
//

#pragma once
#include <string>
#include "handlers/handler.h"

class OrderUpdateHandler :public Handler{
public:
    std::string handle(const http_request &request)  override;
};