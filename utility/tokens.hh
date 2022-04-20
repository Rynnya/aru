#pragma once

#include <drogon/HttpRequest.h>
#include <drogon/Cookie.h>

#include <drogon/utils/coroutine.h>

namespace aru {

    struct token_container {
        int32_t id = 0;
        int64_t permissions = 0;
        std::string token = "";
        std::string token_md5 = "";
    };

    class tokens {
    public:
        static drogon::Task<std::shared_ptr<token_container>> handle_authorization(drogon::HttpRequestPtr req);

    private:
        static drogon::Task<std::shared_ptr<token_container>> authorize_through_token(std::string token);
    };
}