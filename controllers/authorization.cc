#include "authorization.hh"

#include "../utility/helpers.hh"
#include "../utility/tokens.hh"

#include <regex>

Task<HttpResponsePtr> aru::authorization::authorize_user(HttpRequestPtr req) {
    std::shared_ptr<aru::token_container> token_object = co_await aru::tokens::handle_authorization(req);
    
    if (token_object != nullptr) {
        auto db = drogon::app().getDbClient();
        db->execSqlAsync(
            "UPDATE tokens SET last_updated = ? WHERE token = ?;",
            [](const drogon::orm::Result&) {},
            [](const drogon::orm::DrogonDbException&) {},
            aru::utils::get_epoch_time(), token_object->token_md5
        );

        Json::Value metadata = Json::objectValue;
        metadata["id"] = token_object->id;

        auto response = HttpResponse::newHttpJsonResponse(metadata);
        trantor::Date expire = trantor::Date();

        drogon::Cookie cookie = drogon::Cookie("hat", token_object->token);
        cookie.setPath("/");
        cookie.setDomain(aru::config::get()->frontend_link);
        cookie.setExpiresDate(expire.after(1209600));
        cookie.setHttpOnly(true);
        cookie.setSecure(true);

        response->addCookie(cookie);
        co_return response;
    }

    std::shared_ptr<Json::Value> body = req->getJsonObject();

    if (body == nullptr) {
        co_return aru::utils::create_error(k400BadRequest, "invalid json body");
    }

    auto request = *body;

    if (!request["username"].isString() || !request["password"].isString()) {
        co_return aru::utils::create_error(k400BadRequest, "missing required information (username, password)");
    }

    std::string username = request["username"].asString();
    std::string password = request["password"].asString();

    aru::utils::clearify(username);
    auto db = drogon::app().getDbClient();

    int32_t user_id = 0;
    std::string password_hash;
    std::string salt;

    {
        const auto& result = co_await db->execSqlCoro("SELECT id, password_hash, salt FROM users WHERE username = ? LIMIT 1;", username);

        if (result.empty()) {
            co_return aru::utils::create_error(k403Forbidden, "wrong login");
        }

        const auto& row = result.front();

        user_id = row["id"].as<int32_t>();
        password_hash = row["password_hash"].as<std::string>();
        salt = row["salt"].as<std::string>();
    }

    if (user_id == 1) {
        co_return aru::utils::create_error(k403Forbidden, "you cannot login as shiro");
    }

    if (aru::utils::create_sha512(aru::utils::create_md5(password), salt) != password_hash) {
        co_return aru::utils::create_error(k403Forbidden, "wrong login");
    }

    db->execSqlAsync(
        "DELETE FROM tokens WHERE user = ? AND private = ?;",
        [](const drogon::orm::Result&) {},
        [](const drogon::orm::DrogonDbException&) {},
        user_id, true
    );

    std::string token, md5_token;
    while (true) {
        token = aru::utils::generate_random_string(25);
        md5_token = aru::utils::create_md5(token);

        const auto& result = co_await db->execSqlCoro("SELECT id FROM tokens WHERE token = ? LIMIT 1;", md5_token);
        if (result.empty()) {
            db->execSqlAsync(
                "INSERT INTO tokens SET (user_id, token, private, permissions, last_updated) VALUES (?, ?, ?, ?, ?)",
                [](const drogon::orm::Result&) {},
                [](const drogon::orm::DrogonDbException&) {},
                user_id, md5_token, true, 0, aru::utils::get_epoch_time()
            );
            break;
        }
    }

    Json::Value metadata = Json::objectValue;
    metadata["id"] = user_id;

    auto response = HttpResponse::newHttpJsonResponse(metadata);

    drogon::Cookie cookie = drogon::Cookie("hat", token);
    cookie.setPath("/");
    cookie.setDomain(aru::config::get()->frontend_link);
    cookie.setExpiresDate(trantor::Date::now().after(1209600));
    cookie.setHttpOnly(true);
    cookie.setSecure(true);

    response->addCookie(cookie);
    co_return response;
}

Task<HttpResponsePtr> aru::authorization::register_user(HttpRequestPtr req) {
    std::shared_ptr<Json::Value> body = req->getJsonObject();

    if (body == nullptr) {
        co_return aru::utils::create_error(k400BadRequest, "invalid json body");
    }

    auto request = *body;

    if (!request["username"].isString() || !request["password"].isString() || !request["email"].isString()) {
        co_return aru::utils::create_error(k400BadRequest, "missing required information (username, password, email)");
    }

    std::string username = request["username"].asString();
    aru::utils::trim(username);

    std::string safe_username = username;
    aru::utils::clearify(safe_username);

    static std::regex user_regex("^[A-Za-z0-9 _[\\]-]{2,15}$");

    if (!std::regex_search(username, user_regex)) {
        co_return aru::utils::create_error(k403Forbidden, 521, "this nickname contains forbidden symbols. allowed symbols: a-Z 0-9 _[]-");
    }

    if (username.find('_') != std::string::npos && username.find(' ') != std::string::npos) {
        co_return aru::utils::create_error(k403Forbidden, 522, "nickname should not contain spaces and underscores at the same time");
    }

    if (std::any_of(
        aru::config::get()->forbidden_nicknames.begin(),
        aru::config::get()->forbidden_nicknames.end(),
        [&safe_username](const std::string& nickname) { return nickname == safe_username; }
    )) {
        co_return aru::utils::create_error(k403Forbidden, 523, "this nickname is forbidden, if you are real owner of this nickname, please contact us");
    }

    std::string email = request["email"].asString();
    auto db = drogon::app().getDbClient();

    {
        const auto& result = co_await db->execSqlCoro("SELECT id FROM users WHERE safe_username = ? OR username = ? LIMIT 1;", safe_username, username);
        if (!result.empty()) {
            co_return aru::utils::create_error(k403Forbidden, 524, "this nickname already taken");
        }
    }

    {
        const auto& result = co_await db->execSqlCoro("SELECT id FROM users WHERE email = ? LIMIT 1;", email);
        if (!result.empty()) {
            co_return aru::utils::create_error(k403Forbidden, 525, "this email already taken");
        }
    }

    std::string salt = aru::utils::generate_random_string(24);
    std::string password = aru::utils::create_sha512(aru::utils::create_md5(request["password"].asString()), salt);
    int32_t user_id = 0;

    {
        const auto& result = co_await db->execSqlCoro(
            "INSERT INTO users (username, safe_username, country, email, password_hash, salt, ip, registration_date, roles) "
            "VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?);",
            username, safe_username, "XX", email, password, salt, req->getPeerAddr().toIp(), aru::utils::get_epoch_time(), 0
        );
        user_id = result.insertId();
    }

    db->execSqlAsync(
        "INSERT INTO users_stats (id) VALUES (?);",
        [](const drogon::orm::Result&) {},
        [](const drogon::orm::DrogonDbException&) {},
        user_id
    );
    db->execSqlAsync(
        "INSERT INTO users_stats_relax (id) VALUES (?);",
        [](const drogon::orm::Result&) {},
        [](const drogon::orm::DrogonDbException&) {},
        user_id
    );
    db->execSqlAsync(
        "INSERT INTO users_preferences (id) VALUES (?);",
        [](const drogon::orm::Result&) {},
        [](const drogon::orm::DrogonDbException&) {},
        user_id
    );

    std::string token, md5_token;
    while (true) {
        token = aru::utils::generate_random_string(25);
        md5_token = aru::utils::create_md5(token);

        const auto& result = co_await db->execSqlCoro("SELECT id FROM tokens WHERE token = ? LIMIT 1;", md5_token);
        if (result.empty()) {
            db->execSqlAsync(
                "INSERT INTO tokens SET (user_id, token, private, permissions, last_updated) VALUES (?, ?, ?, ?, ?);",
                [](const drogon::orm::Result&) {},
                [](const drogon::orm::DrogonDbException&) {},
                user_id, md5_token, true, 0, aru::utils::get_epoch_time()
            );
            break;
        }
    }

    Json::Value metadata = Json::objectValue;
    metadata["id"] = user_id;

    auto response = HttpResponse::newHttpJsonResponse(metadata);

    drogon::Cookie cookie = drogon::Cookie("hat", token);
    cookie.setPath("/");
    cookie.setDomain(aru::config::get()->frontend_link);
    cookie.setExpiresDate(trantor::Date::now().after(1209600));
    cookie.setHttpOnly(true);
    cookie.setSecure(true);

    response->addCookie(cookie);
    co_return response;
}
