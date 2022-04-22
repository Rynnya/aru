#include "beatmaps.hh"

#include "../utility/helpers.hh"

Task<HttpResponsePtr> aru::beatmaps::retreave_beatmapset(HttpRequestPtr req, int32_t id) {
    auto db = app().getDbClient();
    const auto& result = co_await db->execSqlCoro(
        "SELECT beatmap_id, beatmapset_id, beatmap_md5, "
        "artist, title, difficulty_name, creator, "
        "count_normal, count_slider, count_spinner, "
        "max_combo, ranked_status, latest_update, bpm, hit_length, "
        "difficulty_std, difficulty_taiko, difficulty_ctb, difficulty_mania, "
        "mode, cs, ar, od, hp "
        "FROM beatmaps WHERE beatmapset_id = ? LIMIT 1;", id
    );
    Json::Value beatmaps = Json::arrayValue;

    if (result.empty()) {
        co_return HttpResponse::newHttpJsonResponse(beatmaps);
    }

    for (const auto& row : result) {
        Json::Value beatmap = Json::objectValue;

        beatmap["beatmap_id"]       = row["beatmap_id"].as<int32_t>();
        beatmap["beatmapset_id"]    = row["beatmapset_id"].as<int32_t>();
        beatmap["beatmap_md5"]      = row["beatmap_md5"].as<std::string>();
        beatmap["artist"]           = row["artist"].as<std::string>();
        beatmap["title"]            = row["title"].as<std::string>();
        beatmap["version"]          = row["difficulty_name"].as<std::string>();
        beatmap["creator"]          = row["creator"].as<std::string>();
        beatmap["count_normal"]     = row["count_normal"].as<int32_t>();
        beatmap["count_slider"]     = row["count_slider"].as<int32_t>();
        beatmap["count_spinner"]    = row["count_spinner"].as<int32_t>();
        beatmap["max_combo"]        = row["max_combo"].as<int32_t>();
        beatmap["ranked_status"]    = row["ranked_status"].as<int32_t>();
        beatmap["latest_update"]    = aru::utils::time_to_string(row["latest_update"].as<int64_t>());
        beatmap["bpm"]              = static_cast<Json::Int64>(row["bpm"].as<int64_t>());
        beatmap["hit_length"]       = row["hit_length"].as<int32_t>();

        const int32_t mode = row["mode"].as<int32_t>();
        switch (mode) {
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

        beatmap["cs"] = row["cs"].as<float>();
        beatmap["ar"] = row["ar"].as<float>();
        beatmap["od"] = row["od"].as<float>();
        beatmap["hp"] = row["hp"].as<float>();
        beatmap["mode"] = mode;

        beatmaps.append(beatmap);
    }
    
    co_return HttpResponse::newHttpJsonResponse(beatmaps);
}

Task<HttpResponsePtr> aru::beatmaps::retreave_beatmap(HttpRequestPtr req, int32_t id) {
    auto db = app().getDbClient();
    const auto& result = co_await db->execSqlCoro(
        "SELECT beatmap_id, beatmapset_id, beatmap_md5, "
        "artist, title, difficulty_name, creator, "
        "count_normal, count_slider, count_spinner, "
        "max_combo, ranked_status, latest_update, bpm, hit_length, "
        "difficulty_std, difficulty_taiko, difficulty_ctb, difficulty_mania, "
        "mode, cs, ar, od, hp "
        "FROM beatmaps WHERE beatmap_id = ? LIMIT 1;", id
    );
    Json::Value beatmap = Json::objectValue;

    if (result.empty()) {
        co_return aru::utils::create_error(k404NotFound, "beatmap not found");
    }

    const auto& row = result.front();
    beatmap["beatmap_id"]       = row["beatmap_id"].as<int32_t>();
    beatmap["beatmapset_id"]    = row["beatmapset_id"].as<int32_t>();
    beatmap["beatmap_md5"]      = row["beatmap_md5"].as<std::string>();
    beatmap["artist"]           = row["artist"].as<std::string>();
    beatmap["title"]            = row["title"].as<std::string>();
    beatmap["version"]          = row["difficulty_name"].as<std::string>();
    beatmap["creator"]          = row["creator"].as<std::string>();
    beatmap["count_normal"]     = row["count_normal"].as<int32_t>();
    beatmap["count_slider"]     = row["count_slider"].as<int32_t>();
    beatmap["count_spinner"]    = row["count_spinner"].as<int32_t>();
    beatmap["max_combo"]        = row["max_combo"].as<int32_t>();
    beatmap["ranked_status"]    = row["ranked_status"].as<int32_t>();
    beatmap["latest_update"]    = aru::utils::time_to_string(row["latest_update"].as<int64_t>());
    beatmap["bpm"]              = static_cast<Json::Int64>(row["bpm"].as<int64_t>());
    beatmap["hit_length"]       = row["hit_length"].as<int32_t>();

    const int32_t mode = row["mode"].as<int32_t>();
    switch (mode) {
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

    beatmap["cs"] = row["cs"].as<float>();
    beatmap["ar"] = row["ar"].as<float>();
    beatmap["od"] = row["od"].as<float>();
    beatmap["hp"] = row["hp"].as<float>();
    beatmap["mode"] = mode;

    co_return HttpResponse::newHttpJsonResponse(beatmap);
}

Task<HttpResponsePtr> aru::beatmaps::beatmap_leaderboard(HttpRequestPtr req, int32_t id) {
    auto [mode, is_relax, page, length] = aru::utils::get_parameters<int32_t, bool, uint32_t, uint32_t>(req->getParameters(), "mode", "relax", "page", "length");
    aru::utils::sanitize(mode, 0, 3, 0);

    if (mode == 3 && is_relax) {
        co_return aru::utils::create_error(k400BadRequest, "mania don't have relax mode");
    }

    auto db = app().getDbClient();
    std::string beatmap_md5;

    // Poping database query out of common context, so we can reuse variables
    {
        const auto& result = co_await db->execSqlCoro("SELECT beatmap_md5 FROM beatmaps WHERE beatmap_id = ? LIMIT 1;", id);

        if (result.empty()) {
            co_return aru::utils::create_error(k404NotFound, "beatmap not found");
        }

        const auto& row = result.front();
        beatmap_md5 = row["beatmap_md5"].as<std::string>();
    }

    const auto& result = co_await db->execSqlCoro(
        "SELECT user_id, ranking, username, country, "
        "hash, score, pp, accuracy, count_300, count_100, count_50, count_misses, max_combo, mods "
        "FROM scores JOIN users ON scores.user_id = users.id "
        "WHERE beatmap_md5 = ? AND scores.is_relax = ? AND scores.play_mode = ? AND completed = ?;",
        beatmap_md5, is_relax, mode, true
    );

    if (result.empty()) {
        Json::Value leaderboard_ = Json::arrayValue;
        co_return HttpResponse::newHttpJsonResponse(leaderboard_);
    }

    std::vector<leaderboard_beatmap> leaderboard {};

    for (const auto& row : result) {
        leaderboard_beatmap score;

        score.user_id = row["user_id"].as<int32_t>();
        score.rank = row["ranking"].as<std::string>();
        score.hash = row["hash"].as<std::string>();
        score.username = row["username"].as<std::string>();
        score.country = row["country"].as<std::string>();
        score.score = row["score"].as<int64_t>();
        score.pp = row["pp"].as<float>();
        score.accuracy = row["accuracy"].as<float>();
        score.count_300 = row["count_300"].as<int32_t>();
        score.count_100 = row["count_100"].as<int32_t>();
        score.count_50 = row["count_50"].as<int32_t>();
        score.count_miss = row["count_misses"].as<int32_t>();
        score.max_combo = row["max_combo"].as<int32_t>();
        score.mods = row["mods"].as<int64_t>();

        leaderboard.push_back(std::move(score));
    }

    leaderboard.erase(std::remove_if(leaderboard.begin(), leaderboard.end(), [&](const leaderboard_beatmap& this_) {
        for (const leaderboard_beatmap& that_ : leaderboard) {
            if (this_.hash == that_.hash) {
                continue;
            }

            if (this_.user_id == that_.user_id && that_.score > this_.score) {
                return true;
            }
        }

        return false;
    }), leaderboard.end());

    Json::Value response = Json::arrayValue;
    auto [offset, limit] = aru::utils::paginate(page, length);

    if (leaderboard.size() <= offset) {
        co_return HttpResponse::newHttpJsonResponse(response);
    }

    std::sort(leaderboard.begin(), leaderboard.end(), [](const leaderboard_beatmap& left, const leaderboard_beatmap& right) { return left.score > right.score; });

    for (uint32_t i = offset; i < offset + limit && i < leaderboard.size(); i++) {
        Json::Value score;
        leaderboard_beatmap& this_ = leaderboard.at(i);

        score["user_id"] = this_.user_id;
        score["ranking"] = this_.rank;
        score["hash"] = this_.hash;
        score["username"] = this_.username;
        score["country"] = this_.country;
        score["score"] = this_.score;
        score["pp"] = this_.pp;
        score["accuracy"] = this_.accuracy;
        score["count_300"] = this_.count_300;
        score["count_100"] = this_.count_100;
        score["count_50"] = this_.count_50;
        score["count_misses"] = this_.count_miss;
        score["max_combo"] = this_.max_combo;
        score["mods"] = this_.mods;

        response.append(score);
    }

    co_return HttpResponse::newHttpJsonResponse(response);
}

Task<HttpResponsePtr> aru::beatmaps::ranking_set(HttpRequestPtr req, int32_t id) {
    co_return co_await aru::beatmaps::perform_ranking(req, id, true);
}

Task<HttpResponsePtr> aru::beatmaps::ranking_map(HttpRequestPtr req, int32_t id) {
    co_return co_await aru::beatmaps::perform_ranking(req, id, false);
}

Task<HttpResponsePtr> aru::beatmaps::perform_ranking(HttpRequestPtr req, int32_t id, bool is_set) {
    // TODO: Implement it when direct will be fully done
    co_return aru::utils::create_error(k501NotImplemented, "currently not implemented");
}