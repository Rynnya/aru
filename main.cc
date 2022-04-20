#include <drogon/HttpAppFramework.h>

#include "utility/helpers.hh"

drogon::HttpResponsePtr error_handler(drogon::HttpStatusCode code) {
    auto response = drogon::HttpResponse::newHttpResponse();
    response->setContentTypeCodeAndCustomString(drogon::ContentType::CT_TEXT_PLAIN, "text/plain; charset=utf-8");
    response->setBody("unhandled error, please report me");
    response->setStatusCode(code);
    return response;
}

void default_handler(const drogon::HttpRequestPtr& req, std::function<void(const drogon::HttpResponsePtr &)>&& callback) {
    auto response = drogon::HttpResponse::newHttpResponse();
    response->setContentTypeCodeAndCustomString(drogon::ContentType::CT_TEXT_PLAIN, "text/plain; charset=utf-8");
    response->setBody(
        "aru, yukime backend api\n"
        "source code: https://github.com/Rynnya/aru"
    );
    callback(response);
};

void favicon_handler(const drogon::HttpRequestPtr& req, std::function<void(const drogon::HttpResponsePtr &)>&& callback) {
    drogon::HttpResponsePtr response = drogon::HttpResponse::newHttpResponse();
    response->setStatusCode(drogon::k204NoContent);
    response->setBody("");
    callback(response);
}

int main() {
    drogon::app()
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

    std::thread([] {
        std::this_thread::sleep_for(std::chrono::seconds(1));
        for (auto& listener : drogon::app().getListeners()) {
            LOG_INFO << "Listening on " << listener.toIp() << ":" << listener.toPort();
        }
    }).detach();

    drogon::app().run();
}