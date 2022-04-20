#pragma once

#include <drogon/HttpController.h>
#include <drogon/utils/coroutine.h>

using namespace drogon;
namespace aru {
    class beatmaps : public drogon::HttpController<beatmaps> {
    public:
        Task<HttpResponsePtr> retreave_beatmapset   (HttpRequestPtr req, int32_t id);
        Task<HttpResponsePtr> retreave_beatmap      (HttpRequestPtr req, int32_t id);
        Task<HttpResponsePtr> beatmap_leaderboard   (HttpRequestPtr req, int32_t id);
        Task<HttpResponsePtr> perform_ranking       (HttpRequestPtr req, int32_t id);

        METHOD_LIST_BEGIN
            ADD_METHOD_TO(beatmaps::retreave_beatmapset, "/beatmapset/{1}", Get);
            ADD_METHOD_TO(beatmaps::retreave_beatmap, "/beatmap/{1}", Get);
            ADD_METHOD_TO(beatmaps::beatmap_leaderboard, "/beatmap/{1}/leaderboard", Get);
            ADD_METHOD_TO(beatmaps::perform_ranking, "/beatmap/{1}/ranking", Post);
        METHOD_LIST_END
    };
}
