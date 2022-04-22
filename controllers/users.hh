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
            ADD_METHOD_TO(users::retreave_metadata, "/v1/users/{1}", Get);
            ADD_METHOD_TO(users::retreave_full_metadata, "/v1/users/{1}/full", Get);

            ADD_METHOD_TO(users::retreave_best_scores, "/v1/users/{1}/scores/best", Get);
            ADD_METHOD_TO(users::retreave_recent_scores, "/v1/users/{1}/scores/recent", Get);
            ADD_METHOD_TO(users::retreave_first_scores, "/v1/users/{1}/scores/first", Get);
        METHOD_LIST_END
    private:
        class beatmap_container {
        public:
            beatmap_container() = default;

            beatmap_container(const beatmap_container& other) = delete;
            beatmap_container(beatmap_container&& other) noexcept = default;

            beatmap_container& operator=(const beatmap_container& other) = delete;
            beatmap_container& operator=(beatmap_container&& other) noexcept = default;

            int32_t beatmap_id = 0;
            int32_t beatmapset_id = 0;
            std::string beatmap_md5 {};
            std::string artist {};
            std::string title {};
            std::string difficulty_name {};
            std::string creator {};
            float cs = 0.0f;
            float ar = 0.0f;
            float od = 0.0f;
            float hp = 0.0f;
            int32_t max_combo = 0;
            int32_t hit_length = 0;
            int32_t ranked_status = 0;
            bool ranked_status_frozen = false;
            std::string latest_update {};
            float difficulty = 0.0f;
        };
        class score_container {
        public:
            score_container() = default;

            score_container(const score_container& other) = delete;
            score_container(score_container&& other) noexcept = default;

            score_container& operator=(const score_container& other) = delete;
            score_container& operator=(score_container&& other) noexcept = default;

            int32_t id = 0;
            int32_t user_id = 0;
            std::string hash {};
            std::string rank {};
            int64_t score = 0;
            int32_t max_combo = 0;
            bool full_combo = false;
            int64_t mods = 0;
            int32_t count_300 = 0;
            int32_t count_100 = 0;
            int32_t count_50 = 0;
            int32_t count_geki = 0;
            int32_t count_katu = 0;
            int32_t count_miss = 0;
            std::string date {};
            int64_t epoch = 0;
            int32_t mode = 0;
            float accuracy = 0.0f;
            float pp = 0.0f;
            beatmap_container beatmap {};
        };
        void build_scores(Json::Value& arr, const drogon::orm::Result& records);
    };
}
