#include "handlers/handler.h"

class OrderUserQueryHandler :public Handler{
public:
    std::string handle(const http_request &request)  override;
};