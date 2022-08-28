#include "tokens.hh"

#include <drogon/HttpAppFramework.h>
#include "helpers.hh"

drogon::Task<std::shared_ptr<aru::token_container>> aru::tokens::handle_authorization(drogon::HttpRequestPtr req_) {
    std::string token;

    if ((token = req_->getHeader("Authorization")).empty()) {
        if ((token = req_->getCookie("yukime_session")).empty()) {
            co_return {};
        }
    }

    co_return co_await authorize_through_token(token);
}

drogon::Task<std::shared_ptr<aru::token_container>> aru::tokens::authorize_through_token(std::string token_) {
    auto db = drogon::app().getDbClient();

    const auto record = co_await db->execSqlCoro(
        "SELECT user_id, permissions, token FROM tokens WHERE token = ? AND last_updated + 1209600 > ?;", 
        aru::utils::create_md5(token_), aru::utils::get_epoch_time()
    );

    if (record.empty()) {
        co_return {};
    }

    const auto& result = record.front();
    co_return std::make_shared<aru::token_container>(result["user_id"].as<int32_t>(), result["permissions"].as<int64_t>(), token_, result["token"].as<std::string>());
}