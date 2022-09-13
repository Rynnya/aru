#include <drogon/HttpAppFramework.h>

#include "utility/helpers.hh"

namespace error_types {
    constexpr char method_not_allowed[] = "this method not allowed";
    constexpr char unhandled_error[] = "unhandled error, please report me";
}

drogon::HttpResponsePtr error_handler(drogon::HttpStatusCode code) {
    return aru::utils::create_error(code, code == drogon::k405MethodNotAllowed ? error_types::method_not_allowed : error_types::unhandled_error);
}

void default_handler(const drogon::HttpRequestPtr& req, std::function<void(const drogon::HttpResponsePtr &)>&& callback) {
    auto response = drogon::HttpResponse::newHttpResponse();
    response->setContentTypeCodeAndCustomString(drogon::ContentType::CT_TEXT_PLAIN, "text/plain; charset=utf-8");
    response->setBody("aru, yukime backend api\nsource code: https://github.com/Rynnya/aru");
    callback(response);
};

void favicon_handler(const drogon::HttpRequestPtr& req, std::function<void(const drogon::HttpResponsePtr &)>&& callback) {
    callback(aru::utils::no_content());
}

drogon::HttpResponsePtr options_handler(const drogon::HttpRequestPtr& req) {
    if (req->getMethod() == drogon::HttpMethod::Options) {
        return aru::utils::options();
    }

    return nullptr;
}

void cors_handler(const drogon::HttpRequestPtr& req, const drogon::HttpResponsePtr& resp) {
    resp->addHeader("Access-Control-Allow-Origin", "*");
}

void main_callback() {
    for (auto& listener : drogon::app().getListeners()) {
        LOG_INFO << "Listening on " << listener.toIp() << ":" << listener.toPort();
    }
}

int main() {
    drogon::app()
        .setFloatPrecisionInJson(2, "decimal")
        .registerBeginningAdvice(main_callback)
        .registerSyncAdvice(options_handler)
        .registerPostHandlingAdvice(cors_handler)
        .setCustomErrorHandler(error_handler)
        .setDefaultHandler(default_handler)
        .registerHandler("/favicon.ico", &favicon_handler)
        .loadConfigFile("config.json");

    Json::Value custom_config = drogon::app().getCustomConfig();

    aru::config cfg(
        custom_config["frontend_link"].asString(),
        custom_config["avatar_folder"].asString(),
        { "peppy", "rrtyui", "cookiezi", "azer", "loctav", "banchobot", "happystick", "doomsday", "sharingan33", "andrea", "cptnxn", "reimu-desu",
        "hvick225", "_index", "my_aim_sucks", "kynan", "rafis", "sayonara-bye", "thelewa", "wubwoofwolf", "millhioref", "tom94", "tillerino", "clsw",
        "spectator", "exgon", "axarious", "angelsim", "recia", "nara", "emperorpenguin83", "bikko", "xilver", "vettel", "kuu01", "_yu68", "tasuke912", "dusk",
        "ttobas", "velperk", "jakads", "jhlee0133", "abcdullah", "yuko-", "entozer", "hdhr", "ekoro", "snowwhite", "osuplayer111", "musty", "nero", "elysion",
        "ztrot", "koreapenguin", "fort", "asphyxia", "niko", "shigetora", "whitecat", "fokabot", "himitsu", "nebula", "howl", "nyo", "angelwar", "mm00", "yukime" }
    );

    drogon::app().run();
}