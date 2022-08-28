#include "unspecified.hh"

#include "../utility/helpers.hh"

Task<HttpResponsePtr> aru::unspecified::leaderboard(HttpRequestPtr req) {
    static constexpr const char classic_query[] = 
        "SELECT users.id, username, country, "
        "rank_std, rank_taiko, rank_ctb, rank_mania, "
        "ranked_score_std, ranked_score_taiko, ranked_score_ctb, ranked_score_mania, "
        "avg_accuracy_std, avg_accuracy_taiko, avg_accuracy_ctb, avg_accuracy_mania, "
        "pp_std, pp_taiko, pp_ctb, pp_mania "
        "FROM users JOIN users_stats ON users.id = users_stats.id LIMIT ? OFFSET ?;";
    static constexpr const char relax_query[] =
        "SELECT users.id, username, country, "
        "rank_std, rank_taiko, rank_ctb, "
        "ranked_score_std, ranked_score_taiko, ranked_score_ctb, "
        "avg_accuracy_std, avg_accuracy_taiko, avg_accuracy_ctb, "
        "pp_std, pp_taiko, pp_ctb "
        "FROM users JOIN users_stats_relax ON users.id = users_stats_relax.id LIMIT ? OFFSET ?;";

    auto [mode, is_relax, page, length] = aru::utils::get_parameters<int32_t, bool, uint32_t, uint32_t>(req->getParameters(), "mode", "relax", "page", "length");
    
    if (mode == 3 && is_relax) {
        co_return aru::utils::create_error(k400BadRequest, "mania don't have relax mode");
    }

    auto [offset, limit] = aru::utils::paginate(page, length);
    auto db = app().getDbClient();
    const auto result = co_await db->execSqlCoro(is_relax ? relax_query : classic_query, limit, offset);

    Json::Value leaderboard { Json::arrayValue };
    if (result.empty()) {
        co_return HttpResponse::newHttpJsonResponse(leaderboard);
    }

    for (const auto& row : result) {
        switch (mode) {
            default:
            case 0: {
                const int32_t position = row["rank_std"].as<int32_t>();
                const int64_t score = row["ranked_score_std"].as<int64_t>();
                if (position == 0 || score == 0) {
                    continue;
                }

                Json::Value user { Json::objectValue };
                user["id"] = row["id"].as<int32_t>();
                user["username"] = row["username"].as<std::string>();
                user["country"] = row["country"].as<std::string>();
                user["ranked_score"] = static_cast<Json::Int64>(score);
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

                Json::Value user { Json::objectValue };
                user["id"] = row["id"].as<int32_t>();
                user["username"] = row["username"].as<std::string>();
                user["country"] = row["country"].as<std::string>();
                user["ranked_score"] = static_cast<Json::Int64>(score);
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

                Json::Value user { Json::objectValue };
                user["id"] = row["id"].as<int32_t>();
                user["username"] = row["username"].as<std::string>();
                user["country"] = row["country"].as<std::string>();
                user["ranked_score"] = static_cast<Json::Int64>(score);
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

                Json::Value user { Json::objectValue };
                user["id"] = row["id"].as<int32_t>();
                user["username"] = row["username"].as<std::string>();
                user["country"] = row["country"].as<std::string>();
                user["ranked_score"] = static_cast<Json::Int64>(score);
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