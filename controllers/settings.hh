#pragma once

#include <drogon/HttpController.h>
#include <drogon/utils/coroutine.h>

using namespace drogon;
namespace aru {
    class settings : public drogon::HttpController<settings> {
    public:
        Task<HttpResponsePtr> retreave_settings     (HttpRequestPtr req, int32_t id);

        Task<HttpResponsePtr> retreave_userpage     (HttpRequestPtr req, int32_t id);
        Task<HttpResponsePtr> perform_userpage      (HttpRequestPtr req, int32_t id);

        Task<HttpResponsePtr> retreave_status       (HttpRequestPtr req, int32_t id);
        Task<HttpResponsePtr> perform_status        (HttpRequestPtr req, int32_t id);

        // retreave_avatar can be done by using https://a.example.com/{id}
        Task<HttpResponsePtr> perform_avatar        (HttpRequestPtr req, int32_t id);
        Task<HttpResponsePtr> delete_avatar         (HttpRequestPtr req, int32_t id);

        // others user-specific preferences must be private, as only shiro and website will use then
        Task<HttpResponsePtr> perform_preferences   (HttpRequestPtr req, int32_t id);
        Task<HttpResponsePtr> perform_scoreboard    (HttpRequestPtr req, int32_t id);
        Task<HttpResponsePtr> perform_password      (HttpRequestPtr req, int32_t id);

        METHOD_LIST_BEGIN
            ADD_METHOD_TO(settings::retreave_settings, "/v1/users/{1}/settings", Get);

            ADD_METHOD_TO(settings::retreave_userpage, "/v1/users/{1}/settings/userpage", Get);
            ADD_METHOD_TO(settings::perform_userpage, "/v1/users/{1}/settings/userpage", Put);

            ADD_METHOD_TO(settings::retreave_status, "/v1/users/{1}/settings/status", Get);
            ADD_METHOD_TO(settings::perform_status, "/v1/users/{1}/settings/status", Put);

            ADD_METHOD_TO(settings::perform_avatar, "/v1/users/{1}/settings/avatar", Post);
            ADD_METHOD_TO(settings::delete_avatar, "/v1/users/{1}/settings/avatar", Delete);
            
            ADD_METHOD_TO(settings::perform_preferences, "/v1/users/{1}/settings/preferences", Put);
            ADD_METHOD_TO(settings::perform_scoreboard, "/v1/users/{1}/settings/scoreboard", Put);
            ADD_METHOD_TO(settings::perform_password, "/v1/users/{1}/settings/password", Put);
        METHOD_LIST_END
    private:
        class preferences {
        public:
            preferences() = default;
            preferences(
                bool scoreboard_display_classic, bool scoreboard_display_relax,
                bool score_overwrite_std, bool score_overwrite_taiko,
                bool score_overwrite_ctb, bool score_overwrite_mania,
                bool is_relax
            );
            preferences(uint32_t pref_number);
            bool operator==(const preferences& other);
            bool operator!=(const preferences& other);

            bool scoreboard_display_classic = false;
            bool scoreboard_display_relax = false;
            bool score_overwrite_std = false;
            bool score_overwrite_taiko = false;
            bool score_overwrite_ctb = false;
            bool score_overwrite_mania = false;
            bool is_relax = false;
        };

        Task<std::tuple<bool, std::string>> verify_user(HttpRequestPtr req, int32_t id);
    };
}
