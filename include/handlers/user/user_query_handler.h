#include "handlers/handler.h"

class UserQueryHandler :public Handler{
public:
    std::string handle(const HttpRequest &request)  override;
};