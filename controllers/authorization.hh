#pragma once

#include <drogon/HttpController.h>
#include <drogon/utils/coroutine.h>

using namespace drogon;
namespace aru {
    class authorization : public drogon::HttpController<authorization> {
    public:
        Task<HttpResponsePtr> authorize_user(HttpRequestPtr req);
        Task<HttpResponsePtr> register_user (HttpRequestPtr req);

        METHOD_LIST_BEGIN
            ADD_METHOD_TO(authorization::authorize_user, "/v1/login", Post);
            ADD_METHOD_TO(authorization::register_user, "/v1/register", Post);
        METHOD_LIST_END
    };
}
