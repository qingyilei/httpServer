#include "handlers/handler.h"

class OrderDeleteHandler  :public Handler{
public:
std::string handle(const HttpRequest &request)  override;
};