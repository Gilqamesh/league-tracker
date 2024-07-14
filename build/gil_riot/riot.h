#ifndef RIOT_H
# define RIOT_H

# define GIL_RIOT_VERSION_MAJOR 0
# define GIL_RIOT_VERSION_MINOR 1

# include <string>
# include <functional>

# include "json.hpp"

struct riot_api {
    riot_api() = default;
    riot_api(const std::string& api_key);

    void init(const std::string& api_key);

    std::string m_api_key;

    enum region_t {
        REGION_NA,
        REGION_LAN,
        REGION_EUW,
        REGION_EUNE,
        REGION_OCE,
        REGION_KR,

        _REGION_SIZE
    };
    region_t str_to_region(const std::string& region);
    std::string region_to_str(region_t region);

    enum game_type_t {
        GAME_TYPE_RANKED,
        GAME_TYPE_TUTORIAL,
        GAME_TYPE_NORMAL,
        GAME_TYPE_TOURNEY,

        _GAME_TYPE_SIZE
    };

    void get_puuid_async(
        const std::string& game_name, const std::string& tag_line,
        std::function<void(const std::string& resulting_puuid)> on_success,
        std::function<void()> on_failure
    );

    void get_match_history_async(
        region_t region, game_type_t game_type, const std::string& puuid,
        std::function<void(const nlohmann::json& resulting_match_history)> on_success,
        std::function<void()> on_failure
    );

    void get_match_info_async(
        region_t region, const std::string& match_id,
        std::function<void(const nlohmann::json& resulting_match_info)> on_success,
        std::function<void()> on_failure
    );

    void get_match_timeline_async(
        region_t region, const std::string& match_id,
        std::function<void(const nlohmann::json& resulting_match_timeline)> on_success,
        std::function<void()> on_failure
    );

    void get_challenges_info_async(
        region_t region,
        std::function<void(const nlohmann::json& resulting_challenges_info)> on_success,
        std::function<void()> on_failure
    );

    void get_challenges_by_puuid_async(
        region_t region, const std::string& puuid,
        std::function<void(const nlohmann::json& resulting_challenges_info_for_puuid)> on_success,
        std::function<void()> on_failure
    );
};

void get_liveclientdata_allgamedata_async(
    std::function<void(const nlohmann::json& resulting_live_client_data)> on_success,
    std::function<void()> on_failure
);
void get_liveclientdata_activeplayer_async(
    std::function<void(const nlohmann::json& resulting_live_client_data)> on_success,
    std::function<void()> on_failure
);

#endif // RIOT_H
