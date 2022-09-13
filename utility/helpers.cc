#include "helpers.hh"

#include <openssl/md5.h>
#include <openssl/evp.h>

#include <random>
#include <iomanip>

aru::config* aru::config::instance = nullptr;
std::once_flag aru::config::once {};

drogon::HttpResponsePtr aru::utils::no_content_container = nullptr;

constexpr uint64_t score_to_level[100] = {
        0, 30000, 130000, 340000, 700000,
        1250000, 2030000, 3080000, 4440000, 6150000,
        8250000, 10780000, 13780000, 17290000, 21350000,
        26000000, 31280000, 37230000, 43890000, 51300000,
        59500000, 68530000, 78430000, 89240000, 101000000,
        113750000, 127530000, 142380000, 158340000, 175450000,
        193750000, 213280000, 234080000, 256190000, 279650000,
        304500000, 330780000, 358530000, 387790000, 418600000,
        451000000, 485030000, 520730000, 558140000, 597300000,
        638250000, 681030000, 725680000, 772240000, 820750000,
        871250000, 923780000, 978380000, 1035090000, 1093950000,
        1155000000, 1218280000, 1283830000, 1351690001, 1421900001,
        1494500002, 1569530004, 1647030007, 1727040013, 1809600024,
        1894750043, 1982530077, 2072980138, 2166140248, 2262050446,
        2360750803, 2462281446, 2566682603, 2673994685, 2784258433,
        2897515180, 3013807324, 3133179183, 3255678529, 3381359353,
        3510286835, 3642546304, 3778259346, 3917612824, 4060911082,
        4208669948, 4361785907, 4521840633, 4691649139, 4876246450,
        5084663609, 5333124496, 5650800094, 6090166168, 6745647103,
        7787174786, 9520594614, 12496396305, 17705429349, 26931190829
    };

aru::config::config(const std::string& frontend_link_, const std::string& avatar_folder_, const std::vector<std::string>& forbidden_nicknames_)
    : frontend_link(frontend_link_)
    , avatar_folder(avatar_folder_)
    , forbidden_nicknames(forbidden_nicknames_)
{
    std::call_once(aru::config::once, [&]() {
        aru::utils::no_content_container = drogon::HttpResponse::newHttpResponse();
        aru::utils::no_content_container->setStatusCode(drogon::k204NoContent);
        aru::utils::no_content_container->setContentTypeCode(drogon::CT_APPLICATION_JSON);

        aru::utils::options_container = drogon::HttpResponse::newHttpResponse();
        aru::utils::options_container->addHeader("Access-Control-Allow-Origin", "*");
        aru::utils::options_container->addHeader("Access-Control-Allow-Methods", "GET, POST, PATCH, PUT, DELETE, OPTIONS");

        instance = this;
    });
}

const aru::config* aru::config::get() {
    return instance;
}

const drogon::HttpResponsePtr& aru::utils::no_content() {
    return aru::utils::no_content_container;
}

const drogon::HttpResponsePtr& aru::utils::options() {
    return aru::utils::options_container;
}

drogon::HttpResponsePtr aru::utils::create_error(const drogon::HttpStatusCode& code_, const std::string& reason_) {
    Json::Value object { Json::objectValue };
    object["message"] = reason_;

    auto response = drogon::HttpResponse::newHttpJsonResponse(object);
    response->setStatusCode(code_);

    return response;
}

drogon::HttpResponsePtr aru::utils::create_error(const drogon::HttpStatusCode& code_, int32_t internal_code_, const std::string& reason_) {
    Json::Value object { Json::objectValue };
    object["code"] = internal_code_;
    object["message"] = reason_;

    auto response = drogon::HttpResponse::newHttpJsonResponse(object);
    response->setStatusCode(code_);

    return response;
}

void aru::utils::tolower(std::string& str_) {
    for (auto& ch : str_) {
        ch = std::tolower(ch);
    }
}

void aru::utils::trim(std::string& str_) {
    str_.erase(str_.find_last_not_of(" \t\n\r\f\v") + 1);
    str_.erase(0, str_.find_first_not_of(" \t\n\r\f\v"));
}

void aru::utils::clearify(std::string& str_) {
    aru::utils::tolower(str_);
    aru::utils::trim(str_);
    std::replace(str_.begin(), str_.end(), ' ', '_');
}

std::string aru::utils::create_md5(const std::string& input_) {
    unsigned char result[16];
    char output[33];

    MD5(reinterpret_cast<const unsigned char*>(input_.c_str()), input_.length(), result);

    for (size_t i = 0; i < 16; i++) {
        std::snprintf(&output[i * 2], sizeof(output), "%02x", static_cast<unsigned int>(result[i]));
    }

    return std::string(output);
}

std::string aru::utils::create_sha512(const std::string& input_, const std::string& salt_) {
    unsigned char result[64];
    char output[129];

    PKCS5_PBKDF2_HMAC(input_.c_str(), input_.length(), reinterpret_cast<const unsigned char*>(salt_.c_str()), salt_.size(), 4096, EVP_sha512(), 64, result);

    for (size_t i = 0; i < 64; i++) {
        std::snprintf(&output[i * 2], sizeof(output), "%02x", static_cast<unsigned int>(result[i]));
    }

    return std::string(output);
}

std::string aru::utils::generate_random_string(int32_t size_) {
    std::string result {};
    result.reserve(size_);

    static const char alphanum[] =
        "0123456789"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz";

    for (int32_t i = 0; i < size_; ++i) {
        result.push_back(alphanum[generate_random_number() % (sizeof(alphanum) - 1)]);
    }

    return result;
}

uint32_t aru::utils::generate_random_number() {
    static std::random_device rd;
    static std::mt19937 engine(rd());

    return engine();
}

int64_t aru::utils::get_epoch_time() {
    return std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch()).count();
}

int64_t aru::utils::string_to_time(const Json::Value& time_) {
    if (!time_.isString()) {
        return std::chrono::time_point_cast<std::chrono::seconds>(std::chrono::system_clock::now()).time_since_epoch().count();
    }

    return trantor::Date::fromDbStringLocal(time_.asString()).secondsSinceEpoch();
}

std::string aru::utils::time_to_string(int64_t time_) {
    char buffer[128] = { 0 };
    time_t seconds = static_cast<time_t>(time_);
    struct tm tm_time;

#ifndef _WIN32
    localtime_r(&seconds, &tm_time);
#else
    localtime_s(&tm_time, &seconds);
#endif

    snprintf(buffer, sizeof(buffer), "%4d-%02d-%02d %02d:%02d:%02d",
        tm_time.tm_year + 1900,
        tm_time.tm_mon + 1,
        tm_time.tm_mday,
        tm_time.tm_hour,
        tm_time.tm_min,
        tm_time.tm_sec
    );

    return buffer;
}

std::pair<uint32_t, uint32_t> aru::utils::paginate(uint32_t page_, uint32_t length_) {
    page_ = std::max(page_, 1u);
    aru::utils::sanitize(length_, 1u, 100u, 50u);

    uint32_t start_pos = (page_ - 1) * length_;
    return { start_pos, length_ };
}

void aru::utils::sort(Json::Value& arr_, std::function<bool(const Json::Value&, const Json::Value&)> pred_) {
    if (!arr_.isArray()) {
        return;
    }

    const Json::ArrayIndex size = arr_.size();
    if (size < 2) {
        return;
    }

    for (Json::ArrayIndex it = 0; it < size - 1; it++) {
        for (Json::ArrayIndex jt = it + 1; jt < size; jt++) {
            Json::Value& left = arr_[it];
            Json::Value& right = arr_[jt];
            if (pred_(left, right)) {
                left.swap(right);
            }
        }
    }
}

int32_t aru::osu::get_level(uint64_t score_) {
    if (score_ >= score_to_level[99]) {
        return 100 + static_cast<int32_t>((score_ - score_to_level[99]) / 100000000000);
    }

    for (int32_t i = 0; i != 100; i++) {
        if (score_to_level[i] > score_) {
            return i;
        }
    }

    return 1;
}

uint64_t aru::osu::get_required_score_for_level(int32_t level) {
    if (level <= 0) {
        return 0;
    }

    if (level <= 100) {
        return score_to_level[level - 1];
    }

    return score_to_level[99] + 100000000000 * (static_cast<int64_t>(level) - 100);
}

double aru::osu::get_level_precise(uint64_t score) {
    int32_t base_level = get_level(score);
    int64_t base_level_score = get_required_score_for_level(base_level);
    int64_t score_progress = score - base_level_score;
    int64_t score_level_diff = get_required_score_for_level(base_level + 1) - base_level_score;

    double result = static_cast<double>(score_progress) / static_cast<double>(score_level_diff) + static_cast<double>(base_level);
    return aru::math::is_infinite(result) || aru::math::is_nan(result) ? 0 : result;
}