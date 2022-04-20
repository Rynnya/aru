#include "unspecified.hh"

#include "../utility/helpers.hh"

Task<HttpResponsePtr> aru::unspecified::leaderboard(HttpRequestPtr req) {
    auto [mode, is_relax, page, length] = aru::utils::get_parameters<int32_t, bool, uint32_t, uint32_t>(req->getParameters(), "mode", "relax", "page", "length");
    
    Json::Value leaderboard = Json::arrayValue;
    if (mode == 3 && is_relax) {
        co_return HttpResponse::newHttpJsonResponse(leaderboard);
    }

    const std::string database_name = is_relax ? "users_stats_relax" : "users_stats";
    std::pair<uint32_t, uint32_t> pagination = aru::utils::paginate(page, length);

    auto db = app().getDbClient();
    const auto& result = co_await db->execSqlCoro(
        "SELECT users.id, username, country, "
        "rank_std, rank_taiko, rank_ctb, rank_mania, "
        "ranked_score_std, ranked_score_taiko, ranked_score_ctb, ranked_score_mania, "
        "avg_accuracy_std, avg_accuracy_taiko, avg_accuracy_ctb, avg_accuracy_mania, "
        "pp_std, pp_taiko, pp_ctb, pp_mania "
        "FROM users JOIN " + database_name + " ON users.id = " + database_name + ".id LIMIT ? OFFSET ?;",
        pagination.second, pagination.first
    );

    if (result.empty()) {
        co_return HttpResponse::newHttpJsonResponse(leaderboard);
    }

    // Workaround for GCC
    typedef long long int64_t;

    for (const auto& row : result) {
        switch (mode) {
            default:
            case 0: {
                const int32_t position = row["rank_std"].as<int32_t>();
                const int64_t score = row["ranked_score_std"].as<int64_t>();
                if (position == 0 || score == 0) {
                    continue;
                }

                Json::Value user = Json::objectValue;
                user["id"] = row["id"].as<int32_t>();
                user["username"] = row["username"].as<std::string>();
                user["country"] = row["country"].as<std::string>();
                user["ranked_score"] = score;
                user["accuracy"] = row["avg_accuracy_std"].as<float>();
                user["pp"] = row["pp_std"].as<int32_t>();
                user["global_rank"] = position;
                leaderboard.append(user);
                break;
            }
            case 1: {
                const int32_t position = row["rank_taiko"].as<int32_t>();
                const int64_t score = row["ranked_score_taiko"].as<int64_t>();
                if (position == 0 || score == 0) {
                    continue;
                }

                Json::Value user = Json::objectValue;
                user["id"] = row["id"].as<int32_t>();
                user["username"] = row["username"].as<std::string>();
                user["country"] = row["country"].as<std::string>();
                user["ranked_score"] = score;
                user["accuracy"] = row["avg_accuracy_taiko"].as<float>();
                user["pp"] = row["pp_taiko"].as<int32_t>();
                user["global_rank"] = position;
                leaderboard.append(user);
                break;
            }
            case 2: {
                const int32_t position = row["rank_ctb"].as<int32_t>();
                const int64_t score = row["ranked_score_ctb"].as<int64_t>();
                if (position == 0 || score == 0) {
                    continue;
                }

                Json::Value user = Json::objectValue;
                user["id"] = row["id"].as<int32_t>();
                user["username"] = row["username"].as<std::string>();
                user["country"] = row["country"].as<std::string>();
                user["ranked_score"] = score;
                user["accuracy"] = row["avg_accuracy_ctb"].as<float>();
                user["pp"] = row["pp_ctb"].as<int32_t>();
                user["global_rank"] = position;
                leaderboard.append(user);
                break;
            }
            case 3: {
                const int32_t position = row["rank_mania"].as<int32_t>();
                const int64_t score = row["ranked_score_mania"].as<int64_t>();
                if (position == 0 || score == 0) {
                    continue;
                }

                Json::Value user = Json::objectValue;
                user["id"] = row["id"].as<int32_t>();
                user["username"] = row["username"].as<std::string>();
                user["country"] = row["country"].as<std::string>();
                user["ranked_score"] = score;
                user["accuracy"] = row["avg_accuracy_mania"].as<float>();
                user["pp"] = row["pp_mania"].as<int32_t>();
                user["global_rank"] = position;
                leaderboard.append(user);
                break;
            }
        }
    }

    aru::utils::sort(leaderboard, [](const Json::Value& left, const Json::Value& right) { return left["global_rank"] > right["global_rank"]; });
    co_return HttpResponse::newHttpJsonResponse(leaderboard);
}