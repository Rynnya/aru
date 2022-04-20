#pragma once

#include <drogon/HttpController.h>
#include <drogon/utils/coroutine.h>

using namespace drogon;
namespace aru {
    class users : public drogon::HttpController<users> {
    public:
        Task<HttpResponsePtr> retreave_metadata     (HttpRequestPtr req, int32_t id);
        Task<HttpResponsePtr> retreave_full_metadata(HttpRequestPtr req, int32_t id);

        Task<HttpResponsePtr> retreave_best_scores  (HttpRequestPtr req, int32_t id);
        Task<HttpResponsePtr> retreave_recent_scores(HttpRequestPtr req, int32_t id);
        Task<HttpResponsePtr> retreave_first_scores (HttpRequestPtr req, int32_t id);

        METHOD_LIST_BEGIN
            ADD_METHOD_TO(users::retreave_metadata, "/users/{1}", Get);
            ADD_METHOD_TO(users::retreave_metadata, "/users/{1}/", Get);
            ADD_METHOD_TO(users::retreave_full_metadata, "/users/{1}/full", Get);

            ADD_METHOD_TO(users::retreave_best_scores, "/users/{1}/scores/best", Get);
            ADD_METHOD_TO(users::retreave_recent_scores, "/users/{1}/scores/recent", Get);
            ADD_METHOD_TO(users::retreave_first_scores, "/users/{1}/scores/first", Get);
        METHOD_LIST_END
    private:
        void build_scores(Json::Value& arr, const drogon::orm::Result& records);
    };
}
