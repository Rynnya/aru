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

        Task<HttpResponsePtr> ranking_set           (HttpRequestPtr req, int32_t id);
        Task<HttpResponsePtr> ranking_map           (HttpRequestPtr req, int32_t id);
    private:
        Task<HttpResponsePtr> perform_ranking       (HttpRequestPtr req, int32_t id, bool is_set);
    public:

        METHOD_LIST_BEGIN
            ADD_METHOD_TO(beatmaps::retreave_beatmapset, "/v1/beatmapset/{1}", Get);
            ADD_METHOD_TO(beatmaps::ranking_set, "/v1/beatmapset/{1}/rank", Post);
            ADD_METHOD_TO(beatmaps::retreave_beatmap, "/v1/beatmap/{1}", Get);
            ADD_METHOD_TO(beatmaps::beatmap_leaderboard, "/v1/beatmap/{1}/leaderboard", Get);
            ADD_METHOD_TO(beatmaps::ranking_map, "/v1/beatmap/{1}/rank", Post);
        METHOD_LIST_END
    private:
        class leaderboard_beatmap {
        public:
            leaderboard_beatmap() = default;

            int32_t user_id = 0;
            std::string rank {};
            std::string hash {};
            std::string username {};
            std::string country {};
            int64_t score = 0;
            float accuracy = 0.0f;
            float pp = 0.0f;
            int32_t count_300 = 0;
            int32_t count_100 = 0;
            int32_t count_50 = 0;
            int32_t count_miss = 0;
            int32_t max_combo = 0;
            int64_t mods = 0;
        };
    };
}
