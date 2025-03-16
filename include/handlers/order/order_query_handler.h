#include "handlers/handler.h"

class OrderQueryHandler :public Handler{
public:
    std::string handle(const http_request &request)  override;
};