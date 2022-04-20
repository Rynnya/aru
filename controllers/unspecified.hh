#pragma once

#include <drogon/HttpController.h>
#include <drogon/utils/coroutine.h>

using namespace drogon;
namespace aru {
    class unspecified : public drogon::HttpController<unspecified> {
    public:
        Task<HttpResponsePtr> leaderboard(HttpRequestPtr req);

        METHOD_LIST_BEGIN
            ADD_METHOD_TO(unspecified::leaderboard, "/leaderboard", Get);
        METHOD_LIST_END
    };
}
