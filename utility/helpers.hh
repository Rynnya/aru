#pragma once

#include <drogon/HttpResponse.h>

#include "convert.hh"

namespace aru {

    class config {
    public:
        config(const std::string& frontend_link, const std::string& avatar_folder, const std::vector<std::string>& forbidden_nicknames);
        static const config* get();

        const std::string frontend_link;
        const std::string avatar_folder;

        const std::vector<std::string> forbidden_nicknames;
    private:
        static config* instance;
        static std::once_flag once;
    };

    class utils {
        friend class config;
    private:
        template <typename T>
        static T get_parameter(const std::unordered_map<std::string, std::string>& params, const std::string& name) {
            static_assert(std::is_default_constructible_v<T>, "T must have default constructor");

            auto it = params.find(name);
            if (it == params.end()) {
                return T {};
            }

            try {
                return aru::convert::from_string<T>(it->second);
            }
            catch (const std::exception& ex) {
                LOG_ERROR << ex.what();
                return T {};
            }
        }
    private:
        static drogon::HttpResponsePtr no_content_container;
        static drogon::HttpResponsePtr options_container;

    public:
        static const drogon::HttpResponsePtr& no_content();
        static const drogon::HttpResponsePtr& options();
        static drogon::HttpResponsePtr create_error(const drogon::HttpStatusCode& code, const std::string& reason);
        static drogon::HttpResponsePtr create_error(const drogon::HttpStatusCode& code, int32_t internal_code, const std::string& reason);

        static void tolower (std::string& str);
        static void trim    (std::string& str);    
        static void clearify(std::string& str);

        static std::string create_md5(const std::string& input);
        static std::string create_sha512(const std::string& input, const std::string& salt);

        static std::string generate_random_string(int32_t size);
        static uint32_t generate_random_number();

        static int64_t get_epoch_time();
        static int64_t string_to_time(const Json::Value& time);
        static std::string time_to_string(int64_t time);

        static std::pair<uint32_t, uint32_t> paginate(uint32_t page, uint32_t length);

        // Slow sorting method for Json
        // Iterators in this library doesn't meet RandomAccessIterator requirement
        static void sort(Json::Value& arr, std::function<bool(const Json::Value&, const Json::Value&)> pred);

        template <typename... Outputs, typename... Inputs>
        static std::tuple<Outputs...> get_parameters(const std::unordered_map<std::string, std::string>& params, Inputs&&... names) {
            return std::make_tuple(get_parameter<Outputs>(params, names)...);
        }

        template <typename T>
        static void sanitize(T& val, const T& lo, const T& hi, const T& def) {
            if (val < lo || val > hi) {
                val = def;
            }
        }
    };

    class math {
    public:
        template <typename T>
        static bool is_infinite(const T& value) {
            static_assert(std::is_arithmetic<T>::value, "T must be arithmetic");
            T max_value = std::numeric_limits<T>::max();
            T min_value = -max_value;

            return !(min_value <= value && value <= max_value);
        }

        template <typename T>
        static bool is_nan(const T& value) {
            // True if NAN
            return value != value;
        }
    };

    class osu {
    public:
        static int32_t get_level(uint64_t score);
        static uint64_t get_required_score_for_level(int32_t level);
        static double get_level_precise(uint64_t score);
    };

}