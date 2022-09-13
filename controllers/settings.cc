#include "settings.hh"

#include "../utility/tokens.hh"
#include "../utility/helpers.hh"

#include <fstream>
#include <filesystem>

Task<HttpResponsePtr> aru::settings::retreave_settings(HttpRequestPtr req, int32_t id) {
    auto [authorized, error_message] = co_await verify_user(req, id);
    if (!authorized) {
        co_return aru::utils::create_error(k403Forbidden, error_message);
    }

    auto db = drogon::app().getDbClient();
    const auto result = co_await db->execSqlCoro(
        "SELECT users.id, users.favourite_mode, users.favourite_relax, users.play_style, users.is_relax, "
        "users_preferences.scoreboard_display_classic, users_preferences.scoreboard_display_relax, "
        "users_preferences.auto_last_classic, users_preferences.auto_last_relax, "
        "users_preferences.score_overwrite_std, users_preferences.score_overwrite_taiko, "
        "users_preferences.score_overwrite_ctb, users_preferences.score_overwrite_mania "
        "FROM users INNER JOIN users_preferences ON (users.id = users_preferences.id) WHERE (users.id = ?) LIMIT 1;", id
    );

    if (result.empty()) {
        co_return aru::utils::create_error(k404NotFound, "user not found, somehow");
    }

    const auto& settings = result.front();
    Json::Value body { Json::objectValue };
    Json::Value score { Json::objectValue };

    body["id"]              = settings["id"].as<int32_t>();
    body["submode"]         = static_cast<int32_t>(settings["is_relax"].as<bool>());
    body["default_mode"]    = settings["favourite_mode"].as<int32_t>();
    body["default_relax"]   = static_cast<int32_t>(settings["favourite_relax"].as<bool>());
    body["play_style"]      = settings["play_style"].as<int32_t>();

    body["scoreboard_display_vanilla"] = static_cast<int32_t>(settings["scoreboard_display_classic"].as<bool>());
    body["scoreboard_display_relax"]   = static_cast<int32_t>(settings["scoreboard_display_relax"].as<bool>());
    body["auto_last_vanilla"]          = settings["auto_last_classic"].as<int32_t>();
    body["auto_last_relax"]            = settings["auto_last_relax"].as<int32_t>();

    score["std"]    = static_cast<int32_t>(settings["score_overwrite_std"].as<bool>());
    score["taiko"]  = static_cast<int32_t>(settings["score_overwrite_taiko"].as<bool>());
    score["ctb"]    = static_cast<int32_t>(settings["score_overwrite_ctb"].as<bool>());
    score["mania"]  = static_cast<int32_t>(settings["score_overwrite_mania"].as<bool>());
    body["overwrite"] = score;
    
    co_return HttpResponse::newHttpJsonResponse(body);
}

Task<HttpResponsePtr> aru::settings::retreave_userpage(HttpRequestPtr req, int32_t id) {
    auto db = drogon::app().getDbClient();
    const auto result = co_await db->execSqlCoro("SELECT userpage FROM users WHERE id = ? LIMIT 1;", id);

    if (result.empty()) {
        co_return aru::utils::create_error(k404NotFound, "user not found");
    }

    auto userpage = result.front();
    Json::Value body { Json::objectValue };
    body["user_id"] = id;
    body["userpage"] = userpage["userpage"].as<std::string>();

    co_return HttpResponse::newHttpJsonResponse(body);
}

Task<HttpResponsePtr> aru::settings::perform_userpage(HttpRequestPtr req, int32_t id) {
    auto [authorized, error_message] = co_await verify_user(req, id);
    if (!authorized) {
        co_return aru::utils::create_error(k403Forbidden, error_message);
    }

    std::shared_ptr<Json::Value> body = req->getJsonObject();

    if (body == nullptr) {
        co_return aru::utils::create_error(k400BadRequest, "invalid json body");
    }

    const auto& request = *body;

    if (!request["userpage"].isString()) {
        co_return aru::utils::create_error(k400BadRequest, "missing required parameters (userpage)");
    }

    std::string new_userpage = request["userpage"].asString();

    auto db = drogon::app().getDbClient();
    co_await db->execSqlCoro("UPDATE users SET userpage = ? WHERE id = ?;", new_userpage, id);

    co_return aru::utils::no_content();
}

Task<HttpResponsePtr> aru::settings::retreave_status(HttpRequestPtr req, int32_t id) {
    auto db = drogon::app().getDbClient();
    const auto result = co_await db->execSqlCoro("SELECT status FROM users WHERE id = ? LIMIT 1;", id);

    if (result.empty()) {
        co_return aru::utils::create_error(k404NotFound, "user not found");
    }

    auto userpage = result.front();
    Json::Value body { Json::objectValue };

    body["user_id"] = id;
    body["status"] = userpage["status"].as<std::string>();

    co_return HttpResponse::newHttpJsonResponse(body);
}

Task<HttpResponsePtr> aru::settings::perform_status(HttpRequestPtr req, int32_t id) {
    auto [authorized, error_message] = co_await verify_user(req, id);
    if (!authorized) {
        co_return aru::utils::create_error(k403Forbidden, error_message);
    }

    std::shared_ptr<Json::Value> body = req->getJsonObject();

    if (body == nullptr) {
        co_return aru::utils::create_error(k400BadRequest, "invalid json body");
    }

    const auto& request = *body;

    if (!request["status"].isString()) {
        co_return aru::utils::create_error(k400BadRequest, "missing required parameters (status)");
    }

    std::string new_status = request["status"].asString();

    auto db = drogon::app().getDbClient();
    co_await db->execSqlCoro("UPDATE users SET status = ? WHERE id = ?;", new_status, id);

    co_return aru::utils::no_content();
}

Task<HttpResponsePtr> aru::settings::perform_avatar(HttpRequestPtr req, int32_t id) {
    auto [authorized, error_message] = co_await verify_user(req, id);
    if (!authorized) {
        co_return aru::utils::create_error(k403Forbidden, error_message);
    }

    std::string avatar_path = aru::config::get()->avatar_folder;
    if (avatar_path.empty()) {
        co_return aru::utils::create_error(k503ServiceUnavailable, "avatar changing currently unavailable");
    }

    avatar_path.replace(avatar_path.find("{}"), sizeof("{}") - 1, std::to_string(id));
    std::string_view new_avatar = req->getBody();

    if (new_avatar.size() <= 0 || new_avatar.size() > (1024 * 1024)) {
        co_return aru::utils::create_error(k400BadRequest, "invalid avatar size (must be more than 0 bytes and less than 1 mb)");
    }

    std::ofstream avatar;
    avatar.open(avatar_path, std::ios::binary | std::ios::trunc);

    if (avatar.fail()) {
        co_return aru::utils::create_error(k500InternalServerError, "cannot open avatar file, please report this");
    }

    avatar << new_avatar;
    avatar.close();

    co_await drogon::app().getRedisClient()->execCommandCoro("PUBLISH aru.change_avatar %i", id);
    co_return aru::utils::no_content();
}

Task<HttpResponsePtr> aru::settings::delete_avatar(HttpRequestPtr req, int32_t id) {
    auto [authorized, error_message] = co_await verify_user(req, id);
    if (!authorized) {
        co_return aru::utils::create_error(k403Forbidden, error_message);
    }

    std::string avatar_path = aru::config::get()->avatar_folder;
    if (avatar_path.empty()) {
        co_return aru::utils::create_error(k503ServiceUnavailable, "avatar changing currently unavailable");
    }

    avatar_path.replace(avatar_path.find("{}"), sizeof("{}") - 1, std::to_string(id));
    std::error_code ec;
    std::filesystem::remove(avatar_path, ec);

    if (ec) {
        LOG_FATAL << "aru::settings::delete_avatar(): " << ec.value() << " - " << ec.message();
        co_return aru::utils::create_error(k500InternalServerError, "cannot delete avatar file, please report this");
    }

    co_return aru::utils::no_content();
}

Task<HttpResponsePtr> aru::settings::perform_preferences(HttpRequestPtr req, int32_t id) {
    auto [authorized, error_message] = co_await verify_user(req, id);
    if (!authorized) {
        co_return aru::utils::create_error(k403Forbidden, error_message);
    }

    std::shared_ptr<Json::Value> body = req->getJsonObject();

    if (body == nullptr) {
        co_return aru::utils::create_error(k400BadRequest, "invalid json body");
    }

    bool mode_changed = false;
    bool relax_changed = false;
    bool play_style_changed = false;

    int32_t favourite_mode = -1;
    bool favourite_relax = false;
    int32_t play_style = -1;

    auto db = app().getDbClient();
    auto request = *body;

    const auto result = co_await db->execSqlCoro("SELECT favourite_mode, favourite_relax, play_style FROM users WHERE id = ? LIMIT 1;", id);
    if (result.empty()) {
        co_return aru::utils::create_error(k404NotFound, "user not found, somehow");
    }

    const auto& row = result.front();
    int32_t current_favourite_mode = row["favourite_mode"].as<int32_t>();
    bool current_favourite_relax = row["favourite_relax"].as<bool>();
    int32_t current_play_style = row["play_style"].as<int32_t>();

    if (request["favourite_mode"].isInt()) {
        favourite_mode = request["favourite_mode"].asInt();
        aru::utils::sanitize(favourite_mode, 0, 3, current_favourite_mode);

        mode_changed = current_favourite_mode != favourite_mode;
    }

    if (request["favourite_relax"].isInt() || request["favourite_relax"].isBool()) {
        favourite_relax = request["favourite_relax"].asBool();
        
        relax_changed = favourite_relax != current_favourite_relax;
    }

    if (request["play_style"].isInt()) {
        play_style = request["play_style"].asInt();
        aru::utils::sanitize(play_style, 0, 15, current_play_style);

        play_style_changed = play_style != current_play_style;
    }

    if (!mode_changed && !relax_changed && !play_style_changed) {
        co_return aru::utils::create_error(k304NotModified, "current values are the same");
    }

    if (mode_changed && relax_changed && favourite_mode == 3 && favourite_relax) {
        co_return aru::utils::create_error(k400BadRequest, "mania don't have relax mode");
    }

    // I'm really hate that thing that mania don't have relax
    if (
        (mode_changed && favourite_mode == 3 && !relax_changed && current_favourite_relax) ||
        (relax_changed && favourite_relax && !mode_changed && current_favourite_mode == 3))
    {
        co_return aru::utils::create_error(k400BadRequest, "mania don't have relax mode");
    }

    co_await db->execSqlCoro(
        "UPDATE users SET favourite_mode = ?, favourite_relax = ?, play_style = ? WHERE id = ?;",
        mode_changed ? favourite_mode : current_favourite_mode,
        relax_changed ? favourite_relax : current_favourite_relax,
        play_style_changed ? play_style : current_play_style,
        id
    );

    co_return aru::utils::no_content();
}

Task<HttpResponsePtr> aru::settings::perform_scoreboard(HttpRequestPtr req, int32_t id) {
    auto [authorized, error_message] = co_await verify_user(req, id);
    if (!authorized) {
        co_return aru::utils::create_error(k403Forbidden, error_message);
    }

    std::shared_ptr<Json::Value> body = req->getJsonObject();

    if (body == nullptr) {
        co_return aru::utils::create_error(k400BadRequest, "invalid json body");
    }

    auto db = app().getDbClient();
    const auto& request = *body;

    const auto result = co_await db->execSqlCoro(
        "SELECT users.id, users.is_relax"
        "scoreboard_display_classic, scoreboard_display_relax, "
        "auto_last_classic, auto_last_relax, "
        "score_overwrite_std, score_overwrite_taiko, score_overwrite_ctb, score_overwrite_mania "
        "FROM users_preferences JOIN users ON users_preferences.id = users.id WHERE users.id = ? LIMIT 1;", id
    );
    if (result.empty()) {
        co_return aru::utils::create_error(k404NotFound, "user not found, somehow");
    }

    const auto& row = result.front();

    bool scoreboard_changed = false;
    bool auto_classic_changed = false;
    bool auto_relax_changed = false;

    aru::settings::preferences current_scoreboard {
        row["scoreboard_display_classic"].as<bool>(),
        row["scoreboard_display_relax"].as<bool>(),
        row["score_overwrite_std"].as<bool>(),
        row["score_overwrite_taiko"].as<bool>(),
        row["score_overwrite_ctb"].as<bool>(),
        row["score_overwrite_mania"].as<bool>(),
        row["is_relax"].as<bool>()
    };

    int32_t current_auto_classic = row["auto_last_classic"].as<int32_t>();
    int32_t current_auto_relax = row["auto_last_relax"].as<int32_t>();

    aru::settings::preferences new_scoreboard;
    int32_t auto_classic = 0;
    int32_t auto_relax = 0;

    if (request["preferences"].isInt()) {
        new_scoreboard = aru::settings::preferences(request["preferences"].asInt());

        scoreboard_changed = new_scoreboard != current_scoreboard;
    }

    if (request["auto_last_classic"].isInt()) {
        auto_classic = request["auto_last_classic"].asInt();
        aru::utils::sanitize(auto_classic, 0, 2, current_auto_classic);

        auto_classic_changed = auto_classic != current_auto_classic;
    }

    if (request["auto_last_relax"].isInt()) {
        auto_relax = request["auto_last_relax"].asInt();
        aru::utils::sanitize(auto_relax, 0, 2, current_auto_relax);

        auto_relax_changed = auto_relax != current_auto_relax;
    }

    if (!scoreboard_changed && !auto_classic_changed && !auto_relax_changed) {
        co_return aru::utils::create_error(k304NotModified, "current values are the same");
    }

    if (scoreboard_changed) {
        co_await db->execSqlCoro(
            "UPDATE users_preferences, users INNER JOIN users ON users_preferences.id = users.id "
            "SET scoreboard_display_classic = ?, scoreboard_display_relax = ?, "
            "auto_last_classic = ?, auto_last_relax = ?, "
            "score_overwrite_std = ?, score_overwrite_taiko = ?, score_overwrite_ctb = ?, score_overwrite_mania = ?, "
            "users.is_relax = ? WHERE users.id = ?;",
            new_scoreboard.scoreboard_display_classic, new_scoreboard.scoreboard_display_relax,
            auto_classic_changed ? auto_classic : current_auto_classic, auto_relax_changed ? auto_relax : current_auto_relax,
            new_scoreboard.score_overwrite_std, new_scoreboard.score_overwrite_taiko, new_scoreboard.score_overwrite_ctb, new_scoreboard.score_overwrite_mania,
            new_scoreboard.is_relax, id
        );
        co_return aru::utils::no_content();
    }

    if (auto_classic_changed || auto_relax_changed) {
        co_await db->execSqlCoro(
            "UPDATE users_preferences SET auto_last_classic = ?, auto_last_relax = ? WHERE id = ?;", 
            auto_classic_changed ? auto_classic : current_auto_classic, auto_relax_changed ? auto_relax : current_auto_relax, id
        );
    }

    co_return aru::utils::no_content();
}

Task<HttpResponsePtr> aru::settings::perform_password(HttpRequestPtr req, int32_t id) {
    auto [authorized, error_message] = co_await verify_user(req, id);
    if (!authorized) {
        co_return aru::utils::create_error(k403Forbidden, error_message);
    }

    std::shared_ptr<Json::Value> body = req->getJsonObject();

    if (body == nullptr) {
        co_return aru::utils::create_error(k400BadRequest, "invalid json body");
    }

    auto request = *body;

    if (!request["current_password"].isString() || !request["new_password"].isString()) {
        co_return aru::utils::create_error(k400BadRequest, "invalid json body");
    }

    std::string current_password = request["current_password"].asString();
    std::string new_password = request["new_password"].asString();

    if (current_password == new_password) {
        co_return aru::utils::create_error(k304NotModified, "same password was provided");
    }

    auto db = app().getDbClient();
    const auto result = co_await db->execSqlCoro("SELECT password_hash, salt FROM users WHERE id = ? LIMIT 1;", id);
    if (result.empty()) {
        co_return aru::utils::create_error(k404NotFound, "user not found, somehow");
    }

    const auto& row = result.front();
    std::string current_password_hash = row["password_hash"].as<std::string>();
    std::string current_salt = row["salt"].as<std::string>();

    if (aru::utils::create_sha512(aru::utils::create_md5(current_password), current_salt) != current_password_hash) {
        co_return aru::utils::create_error(k403Forbidden, "provided password isn't actual user password");
    }

    std::string new_salt = aru::utils::generate_random_string(24);
    std::string new_password_hash = aru::utils::create_sha512(aru::utils::create_md5(new_password), new_salt);

    co_await db->execSqlCoro("UPDATE users SET password_hash = ?, salt = ? WHERE id = ?;", new_password_hash, new_salt, id);
    co_return aru::utils::no_content();
}

aru::settings::preferences::preferences(
    bool scoreboard_display_classic_, bool scoreboard_display_relax_,
    bool score_overwrite_std_, bool score_overwrite_taiko_,
    bool score_overwrite_ctb_, bool score_overwrite_mania_,
    bool is_relax_)
    : scoreboard_display_classic(scoreboard_display_classic_)
    , scoreboard_display_relax(scoreboard_display_relax_)
    , score_overwrite_std(score_overwrite_std_)
    , score_overwrite_taiko(score_overwrite_taiko_)
    , score_overwrite_ctb(score_overwrite_ctb_)
    , score_overwrite_mania(score_overwrite_mania_)
    , is_relax(is_relax_)
{}

aru::settings::preferences::preferences(uint32_t pref_number)
    : scoreboard_display_classic((pref_number & 1) > 0)
    , scoreboard_display_relax((pref_number & 2) > 0)
    , score_overwrite_std((pref_number & 4) > 0)
    , score_overwrite_taiko((pref_number & 8) > 0)
    , score_overwrite_ctb((pref_number & 16) > 0)
    , score_overwrite_mania((pref_number & 32) > 0)
    , is_relax((pref_number & 64) > 0)
{}

bool aru::settings::preferences::operator==(const preferences& other) {
    return
        this->scoreboard_display_classic == other.scoreboard_display_classic &&
        this->scoreboard_display_relax == other.scoreboard_display_relax &&
        this->score_overwrite_std == other.score_overwrite_std &&
        this->score_overwrite_taiko == other.score_overwrite_taiko &&
        this->score_overwrite_ctb == other.score_overwrite_ctb &&
        this->score_overwrite_mania == other.score_overwrite_mania &&
        this->is_relax == other.is_relax;
}

bool aru::settings::preferences::operator!=(const preferences& other) {
    return !(this->operator==(other));
}

Task<std::tuple<bool, std::string>> aru::settings::verify_user(HttpRequestPtr req, int32_t id) {
    std::shared_ptr<aru::token_container> token_object = co_await aru::tokens::handle_authorization(req);

    if (token_object == nullptr) {
        co_return { false, "invalid credentials" };
    }

    if (token_object->id != id) {
        LOG_WARN << "User " << token_object->id << " tries to login as " << id;
        co_return { false, "invalid credentials" };
    }

    co_return { true, "" };
}