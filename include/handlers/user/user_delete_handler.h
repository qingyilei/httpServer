#include "handlers/handler.h"

class UserDeleteHandler  :public Handler{
public:
std::string handle(const HttpRequest &request)  override;
};