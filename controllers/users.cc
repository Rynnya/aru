#include "users.hh"

#include "../utility/helpers.hh"

Task<HttpResponsePtr> aru::users::retreave_metadata(HttpRequestPtr req, int32_t id) {
    auto [mode, is_relax] = aru::utils::get_parameters<int32_t, bool>(req->getParameters(), "mode", "relax");

    if (mode == 3 && is_relax) {
        co_return aru::utils::create_error(k400BadRequest, "mania don't have relax mode");
    }

    const std::string database_name = is_relax ? "users_stats_relax" : "users_stats";
    auto db = app().getDbClient();

    const auto& result = co_await db->execSqlCoro(
        "SELECT users.id, username, country, "
        "status, favourite_mode, favourite_relax, "
        "rank_std, rank_taiko, rank_ctb, rank_mania, "
        "ranked_score_std, ranked_score_taiko, ranked_score_ctb, ranked_score_mania, "
        "avg_accuracy_std, avg_accuracy_taiko, avg_accuracy_ctb, avg_accuracy_mania, "
        "pp_std, pp_taiko, pp_ctb, pp_mania "
        "play_count_std, play_count_taiko, play_count_ctb, play_count_mania "
        "FROM users JOIN " + database_name + " ON users.id = " + database_name + ".id WHERE users.is_public = true AND users.id = ? LIMIT 1;", id
    );

    if (result.empty()) {
        co_return aru::utils::create_error(k404NotFound, "user not found");
    }

    const auto& row = result.front();
    Json::Value metadata = Json::objectValue;

    // Workaround for GCC
    typedef long long int64_t;

    metadata["id"] = row["id"].as<int32_t>();
    metadata["username"] = row["username"].as<std::string>();
    metadata["country"] = row["country"].as<std::string>();
    metadata["status"] = row["status"].as<std::string>();
    metadata["default_mode"] = row["favourite_mode"].as<int32_t>();
    metadata["default_relax"] = static_cast<int32_t>(row["favourite_relax"].as<bool>());

    switch (mode) {
        default:
        case 0: {
            metadata["global_rank"] = row["rank_std"].as<int32_t>();
            metadata["ranked_score"] = row["ranked_score_std"].as<int64_t>();
            metadata["accuracy"] = row["avg_accuracy_std"].as<float>();
            metadata["pp"] = row["pp_std"].as<int32_t>();
            metadata["play_count"] = row["play_count_std"].as<int32_t>();
            break;
        }
        case 1: {
            metadata["global_rank"] = row["rank_taiko"].as<int32_t>();
            metadata["ranked_score"] = row["ranked_score_taiko"].as<int64_t>();
            metadata["accuracy"] = row["avg_accuracy_taiko"].as<float>();
            metadata["pp"] = row["pp_taiko"].as<int32_t>();
            metadata["play_count"] = row["play_count_taiko"].as<int32_t>();
            break;
        }
        case 2: {
            metadata["global_rank"] = row["rank_ctb"].as<int32_t>();
            metadata["ranked_score"] = row["ranked_score_ctb"].as<int64_t>();
            metadata["accuracy"] = row["avg_accuracy_ctb"].as<float>();
            metadata["pp"] = row["pp_ctb"].as<int32_t>();
            metadata["play_count"] = row["play_count_ctb"].as<int32_t>();
            break;
        }
        case 3: {
            metadata["global_rank"] = row["rank_mania"].as<int32_t>();
            metadata["ranked_score"] = row["ranked_score_mania"].as<int64_t>();
            metadata["accuracy"] = row["avg_accuracy_mania"].as<float>();
            metadata["pp"] = row["pp_mania"].as<int32_t>();
            metadata["play_count"] = row["play_count_mania"].as<int32_t>();
            break;
        }
    }

    co_return HttpResponse::newHttpJsonResponse(metadata);
}

Task<HttpResponsePtr> aru::users::retreave_full_metadata(HttpRequestPtr req, int32_t id) {
    auto [mode, is_relax] = aru::utils::get_parameters<int32_t, bool>(req->getParameters(), "mode", "relax");

    if (mode == 3 && is_relax) {
        co_return aru::utils::create_error(k400BadRequest, "mania don't have relax mode");
    }

    const std::string database_name = is_relax ? "users_stats_relax" : "users_stats";
    auto db = app().getDbClient();

    const auto& result = co_await db->execSqlCoro(
        "SELECT users.id, username, country, registration_date, latest_activity, "
        "status, favourite_mode, favourite_relax, play_style, "
        "rank_std, rank_taiko, rank_ctb, rank_mania, "
        "ranked_score_std, ranked_score_taiko, ranked_score_ctb, ranked_score_mania, "
        "total_score_std, total_score_taiko, total_score_ctb, total_score_mania, "
        "play_count_std, play_count_taiko, play_count_ctb, play_count_mania, "
        "total_hits_std, total_hits_taiko, total_hits_ctb, total_hits_mania, "
        "max_combo_std, max_combo_taiko, max_combo_ctb, max_combo_mania, "
        "play_time_std, play_time_taiko, play_time_ctb, play_time_mania, "
        "avg_accuracy_std, avg_accuracy_taiko, avg_accuracy_ctb, avg_accuracy_mania, "
        "pp_std, pp_taiko, pp_ctb, pp_mania, "
        "play_count_std, play_count_taiko, play_count_ctb, play_count_mania, "
        "count_A_std, count_A_taiko, count_A_ctb, count_A_mania, "
        "count_S_std, count_S_taiko, count_S_ctb, count_S_mania, "
        "count_SH_std, count_SH_taiko, count_SH_ctb, count_SH_mania, "
        "count_X_std, count_X_taiko, count_X_ctb, count_X_mania, "
        "count_XH_std, count_XH_taiko, count_XH_ctb, count_XH_mania "
        "FROM users JOIN " + database_name + " ON users.id = " + database_name + ".id WHERE users.is_public = true AND users.id = ? LIMIT 1;", id
    );

    if (result.empty()) {
        co_return aru::utils::create_error(k404NotFound, "user not found");
    }

    const auto& row = result.front();
    Json::Value metadata = Json::objectValue;

    // Workaround for GCC
    typedef long long int64_t;

    metadata["id"] = row["id"].as<int32_t>();
    metadata["username"] = row["username"].as<std::string>();
    metadata["registration_date"] = aru::utils::time_to_string(row["registration_date"].as<int64_t>());
    metadata["latest_activity"] = aru::utils::time_to_string(row["latest_activity"].as<int64_t>());
    metadata["country"] = row["country"].as<std::string>();
    metadata["status"] = row["status"].as<std::string>();
    metadata["play_style"] = row["play_style"].as<int32_t>();
    metadata["default_mode"] = row["favourite_mode"].as<int32_t>();
    metadata["default_relax"] = static_cast<int32_t>(row["favourite_relax"].as<bool>());

    switch (mode) {
        default:
        case 0: {
            const int64_t score = row["total_score_std"].as<int64_t>();
            metadata["stats"]["global_rank"] = row["rank_std"].as<int32_t>();
            metadata["stats"]["ranked_score"] = row["ranked_score_std"].as<int64_t>();
            metadata["stats"]["total_score"] = score;
            metadata["stats"]["accuracy"] = row["avg_accuracy_std"].as<float>();
            metadata["stats"]["pp"] = row["pp_std"].as<int32_t>();
            metadata["stats"]["play_count"] = row["play_count_std"].as<int32_t>();
            metadata["stats"]["play_time"] = row["play_time_std"].as<int32_t>();
            metadata["stats"]["total_hits"] = row["total_hits_std"].as<int32_t>();
            metadata["stats"]["level"] = aru::osu::get_level_precise(score);
            metadata["stats"]["A"] = row["count_A_std"].as<int32_t>();
            metadata["stats"]["S"] = row["count_S_std"].as<int32_t>();
            metadata["stats"]["SH"] = row["count_SH_std"].as<int32_t>();
            metadata["stats"]["X"] = row["count_X_std"].as<int32_t>();
            metadata["stats"]["XH"] = row["count_XH_std"].as<int32_t>();
            break;
        }
        case 1: {
            const int64_t score = row["total_score_taiko"].as<int64_t>();
            metadata["stats"]["global_rank"] = row["rank_taiko"].as<int32_t>();
            metadata["stats"]["ranked_score"] = row["ranked_score_taiko"].as<int64_t>();
            metadata["stats"]["total_score"] = score;
            metadata["stats"]["accuracy"] = row["avg_accuracy_taiko"].as<float>();
            metadata["stats"]["pp"] = row["pp_taiko"].as<int32_t>();
            metadata["stats"]["play_count"] = row["play_count_taiko"].as<int32_t>();
            metadata["stats"]["play_time"] = row["play_time_taiko"].as<int32_t>();
            metadata["stats"]["total_hits"] = row["total_hits_taiko"].as<int32_t>();
            metadata["stats"]["level"] = aru::osu::get_level_precise(score);
            metadata["stats"]["A"] = row["count_A_taiko"].as<int32_t>();
            metadata["stats"]["S"] = row["count_S_taiko"].as<int32_t>();
            metadata["stats"]["SH"] = row["count_SH_taiko"].as<int32_t>();
            metadata["stats"]["X"] = row["count_X_taiko"].as<int32_t>();
            metadata["stats"]["XH"] = row["count_XH_taiko"].as<int32_t>();
            break;
        }
        case 2: {
            const int64_t score = row["total_score_ctb"].as<int64_t>();
            metadata["stats"]["global_rank"] = row["rank_ctb"].as<int32_t>();
            metadata["stats"]["ranked_score"] = row["ranked_score_ctb"].as<int64_t>();
            metadata["stats"]["total_score"] = score;
            metadata["stats"]["accuracy"] = row["avg_accuracy_ctb"].as<float>();
            metadata["stats"]["pp"] = row["pp_ctb"].as<int32_t>();
            metadata["stats"]["play_count"] = row["play_count_ctb"].as<int32_t>();
            metadata["stats"]["play_time"] = row["play_time_ctb"].as<int32_t>();
            metadata["stats"]["total_hits"] = row["total_hits_ctb"].as<int32_t>();
            metadata["stats"]["level"] = aru::osu::get_level_precise(score);
            metadata["stats"]["A"] = row["count_A_ctb"].as<int32_t>();
            metadata["stats"]["S"] = row["count_S_ctb"].as<int32_t>();
            metadata["stats"]["SH"] = row["count_SH_ctb"].as<int32_t>();
            metadata["stats"]["X"] = row["count_X_ctb"].as<int32_t>();
            metadata["stats"]["XH"] = row["count_XH_ctb"].as<int32_t>();
            break;
        }
        case 3: {
            const int64_t score = row["total_score_mania"].as<int64_t>();
            metadata["stats"]["global_rank"] = row["rank_mania"].as<int32_t>();
            metadata["stats"]["ranked_score"] = row["ranked_score_mania"].as<int64_t>();
            metadata["stats"]["total_score"] = score;
            metadata["stats"]["accuracy"] = row["avg_accuracy_mania"].as<float>();
            metadata["stats"]["pp"] = row["pp_mania"].as<int32_t>();
            metadata["stats"]["play_count"] = row["play_count_mania"].as<int32_t>();
            metadata["stats"]["play_time"] = row["play_time_mania"].as<int32_t>();
            metadata["stats"]["total_hits"] = row["total_hits_mania"].as<int32_t>();
            metadata["stats"]["level"] = aru::osu::get_level_precise(score);
            metadata["stats"]["A"] = row["count_A_mania"].as<int32_t>();
            metadata["stats"]["S"] = row["count_S_mania"].as<int32_t>();
            metadata["stats"]["SH"] = row["count_SH_mania"].as<int32_t>();
            metadata["stats"]["X"] = row["count_X_mania"].as<int32_t>();
            metadata["stats"]["XH"] = row["count_XH_mania"].as<int32_t>();
            break;
        }
    }

    co_return HttpResponse::newHttpJsonResponse(metadata);
}

Task<HttpResponsePtr> aru::users::retreave_best_scores(HttpRequestPtr req, int32_t id) {
    auto [mode, is_relax, page, length] = aru::utils::get_parameters<int32_t, bool, uint32_t, uint32_t>(req->getParameters(), "mode", "relax", "page", "length");
    aru::utils::sanitize(mode, 0, 3, 0);

    if (mode == 3 && is_relax) {
        co_return aru::utils::create_error(k400BadRequest, "mania don't have relax mode");
    }

    auto db = app().getDbClient();

    // Verify if user isn't banned or restricted
    {
        const auto& result = co_await db->execSqlCoro("SELECT is_public FROM users WHERE id = ? AND is_public = 1;", id);
        if (result.empty()) {
            co_return aru::utils::create_error(k404NotFound, "user not found");
        }
    }

    std::pair<uint32_t, uint32_t> pagination = aru::utils::paginate(page, length);
    Json::Value scores = Json::arrayValue;

    const auto& result = co_await db->execSqlCoro(
        "SELECT scores.id, ranking, score, scores.max_combo as user_max_combo, full_combo, mods, "
        "count_300, count_100, count_50, count_gekis, count_katus, count_misses, "
        "time, play_mode, accuracy, pp, "
        "beatmap_id, beatmapset_id, beatmaps.beatmap_md5, "
        "artist, title, difficulty_name, creator, "
        "cs, ar, od, hp, "
        "difficulty_std, difficulty_taiko, difficulty_ctb, difficulty_mania, "
        "beatmaps.max_combo as beatmap_max_combo, hit_length, "
        "ranked_status, ranked_status_freezed, latest_update "
        "FROM scores JOIN beatmaps ON scores.beatmap_md5 = beatmaps.beatmap_md5 "
        "WHERE pp > 0 AND play_mode = ? AND user_id = ? AND is_relax = ? AND completed = ? "
        "LIMIT ? OFFSET ?;",
        mode, id, is_relax, true, pagination.second, pagination.first
    );

    this->build_scores(scores, result);
    aru::utils::sort(scores, [](const Json::Value& left, const Json::Value& right) { return left["pp"] < right["pp"]; });
    co_return HttpResponse::newHttpJsonResponse(scores);
}

Task<HttpResponsePtr> aru::users::retreave_recent_scores(HttpRequestPtr req, int32_t id) {
    auto [mode, is_relax, page, length, show_failed] = 
        aru::utils::get_parameters<int32_t, bool, uint32_t, uint32_t, bool>(req->getParameters(), "mode", "relax", "page", "length", "failed");
    aru::utils::sanitize(mode, 0, 3, 0);

    if (mode == 3 && is_relax) {
        co_return aru::utils::create_error(k400BadRequest, "mania don't have relax mode");
    }

    auto db = app().getDbClient();

    // Verify if user isn't banned or restricted
    {
        const auto& result = co_await db->execSqlCoro("SELECT is_public FROM users WHERE id = ? AND is_public = 1;", id);
        if (result.empty()) {
            co_return aru::utils::create_error(k404NotFound, "user not found");
        }
    }

    std::pair<uint32_t, uint32_t> pagination = aru::utils::paginate(page, length);
    Json::Value scores = Json::arrayValue;

    const auto& result = co_await db->execSqlCoro(
        "SELECT scores.id, ranking, score, scores.max_combo as user_max_combo, full_combo, mods, "
        "count_300, count_100, count_50, count_gekis, count_katus, count_misses, "
        "time, play_mode, accuracy, pp, "
        "beatmap_id, beatmapset_id, beatmaps.beatmap_md5, "
        "artist, title, difficulty_name, creator, "
        "cs, ar, od, hp, "
        "difficulty_std, difficulty_taiko, difficulty_ctb, difficulty_mania, "
        "beatmaps.max_combo as beatmap_max_combo, hit_length, "
        "ranked_status, ranked_status_freezed, latest_update "
        "FROM scores JOIN beatmaps ON scores.beatmap_md5 = beatmaps.beatmap_md5 "
        "WHERE play_mode = ? AND user_id = ? AND is_relax = ? AND (? OR completed = ?) "
        "LIMIT ? OFFSET ?;",
        mode, id, is_relax, show_failed, true, pagination.second, pagination.first
    );

    this->build_scores(scores, result);
    aru::utils::sort(scores, [](const Json::Value& left, const Json::Value& right) { return left["id"] < right["id"]; });
    co_return HttpResponse::newHttpJsonResponse(scores);
}

Task<HttpResponsePtr> aru::users::retreave_first_scores(HttpRequestPtr req, int32_t id) {
    auto [mode, is_relax, page, length] = aru::utils::get_parameters<int32_t, bool, uint32_t, uint32_t>(req->getParameters(), "mode", "relax", "page", "length");
    aru::utils::sanitize(mode, 0, 3, 0);

    if (mode == 3 && is_relax) {
        co_return aru::utils::create_error(k400BadRequest, "mania don't have relax mode");
    }

    auto db = app().getDbClient();

    // Verify if user isn't banned or restricted
    {
        const auto& result = co_await db->execSqlCoro("SELECT is_public FROM users WHERE id = ? AND is_public = 1;", id);
        if (result.empty()) {
            co_return aru::utils::create_error(k404NotFound, "user not found");
        }
    }
    
    std::pair<uint32_t, uint32_t> pagination = aru::utils::paginate(page, length);
    Json::Value scores = Json::arrayValue;

    const auto& result = co_await db->execSqlCoro(
        "SELECT scores.id, ranking, score, scores.max_combo as user_max_combo, full_combo, mods, "
        "count_300, count_100, count_50, count_gekis, count_katus, count_misses, "
        "time, scores.play_mode, accuracy, pp, "
        "beatmap_id, beatmapset_id, beatmaps.beatmap_md5, "
        "artist, title, difficulty_name, creator, "
        "cs, ar, od, hp, "
        "difficulty_std, difficulty_taiko, difficulty_ctb, difficulty_mania, "
        "beatmaps.max_combo as beatmap_max_combo, hit_length, "
        "ranked_status, ranked_status_freezed, latest_update "
        "FROM scores JOIN beatmaps ON scores.beatmap_md5 = beatmaps.beatmap_md5 "
        "JOIN scores_first ON scores.id = scores_first.score_id "
        "WHERE scores.play_mode = ? AND scores_first.user_id = ? AND scores.is_relax = ? AND completed = ? "
        "LIMIT ? OFFSET ?;",
        mode, id, is_relax, true, pagination.second, pagination.first
    );

    this->build_scores(scores, result);
    aru::utils::sort(scores, [](const Json::Value& left, const Json::Value& right) { return left["epoch"] < right["epoch"]; });
    co_return HttpResponse::newHttpJsonResponse(scores);
}

void aru::users::build_scores(Json::Value& arr, const drogon::orm::Result& records) {

    // Workaround for GCC
    typedef long long int64_t;

    for (const auto& row : records) {
        Json::Value score;
        Json::Value beatmap;
        const int64_t time = row["time"].as<int64_t>();

        score["id"] = row["id"].as<int32_t>();
        score["rank"] = row["ranking"].as<std::string>();
        score["score"] = row["score"].as<int64_t>();
        score["max_combo"] = row["user_max_combo"].as<int32_t>();
        score["full_combo"] = row["full_combo"].as<bool>();
        score["mods"] = row["mods"].as<int64_t>();
        score["count_300"] = row["count_300"].as<int32_t>();
        score["count_100"] = row["count_100"].as<int32_t>();
        score["count_50"] = row["count_50"].as<int32_t>();
        score["count_geki"] = row["count_gekis"].as<int32_t>();
        score["count_katu"] = row["count_katus"].as<int32_t>();
        score["count_miss"] = row["count_misses"].as<int32_t>();
        score["date"] = aru::utils::time_to_string(time);
        score["epoch"] = time;
        score["mode"] = row["play_mode"].as<int32_t>();
        score["accuracy"] = row["accuracy"].as<float>();
        score["pp"] = row["pp"].as<int32_t>();

        beatmap["beatmap_id"] = row["beatmap_id"].as<int32_t>();
        beatmap["beatmapset_id"] = row["beatmapset_id"].as<int32_t>();
        beatmap["beatmap_md5"] = row["beatmap_md5"].as<std::string>();
        beatmap["artist"] = row["artist"].as<std::string>();
        beatmap["title"] = row["title"].as<std::string>();
        beatmap["difficulty_name"] = row["difficulty_name"].as<std::string>();
        beatmap["creator"] = row["creator"].as<std::string>();
        beatmap["cs"] = row["cs"].as<float>();
        beatmap["ar"] = row["ar"].as<float>();
        beatmap["od"] = row["od"].as<float>();
        beatmap["hp"] = row["hp"].as<float>();
        beatmap["max_combo"] = row["beatmap_max_combo"].as<int32_t>();
        beatmap["hit_length"] = row["hit_length"].as<int32_t>();
        beatmap["ranked_status"] = row["ranked_status"].as<int32_t>();
        beatmap["ranked_status_frozen"] = row["ranked_status_freezed"].as<int32_t>();
        beatmap["latest_update"] = aru::utils::time_to_string(row["latest_update"].as<int64_t>());

        switch (score["mode"].asInt()) {
            default:
            case 0: {
                beatmap["difficulty"] = row["difficulty_std"].as<float>();
                break;
            }
            case 1: {
                beatmap["difficulty"] = row["difficulty_taiko"].as<float>();
                break;
            }
            case 2: {
                beatmap["difficulty"] = row["difficulty_ctb"].as<float>();
                break;
            }
            case 3: {
                beatmap["difficulty"] = row["difficulty_mania"].as<float>();
                break;
            }
        }

        score["beatmap"] = beatmap;
        arr.append(score);
    }
}