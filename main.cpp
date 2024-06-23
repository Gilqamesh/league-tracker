#include "riot.h"
#include "web.h"
#include "raylib.h"
#define RAYGUI_IMPLEMENTATION
#include "raygui.h"
#include "json.hpp"
#include "config.h"
#include "asset_manager.h"

#include <iostream>
#include <fstream>
#include <cstring>

#define ARRAY_SIZE(arr) (sizeof(arr)/sizeof((arr)[0]))

struct asset_data_json_t : public asset_data_base_t {
    nlohmann::json json;
};

struct challenge_t {
    int                       id;
    bool                      leaderboard;
    std::string               short_description;
    std::string               description;
    std::string               name;
    std::string               state;
    std::string               tier;
    std::string               next_tier;
    double                    percentile;
    double                    value;
    double                    next_value;
    time_t                    achieved_time;
    std::vector<challenge_t*> children;
    challenge_t*              parent;
};

struct {
    float window_w;
    float window_h;

    Font liberation_mono;

    bool is_game_name_text_box_active;
    char game_name_text_box[256];
    bool is_tag_line_text_box_active;
    char tag_line_text_box[256];
    nlohmann::json account_challenges;
    nlohmann::json global_challenges;

    challenge_t* current_challange;
    challenge_t* challanges_root;

    riot_api riot;
    asset_manager_t asset_manager;
} _;

#if defined(PLATFORM_WEB)
static void update_and_draw();
#endif

static int init(int argc, char** argv);
static void update(double dt);
static void draw();
static void draw_challenges();
static void draw_challenge(challenge_t* node, const Rectangle& rec, int is_detailed);
static void draw_challenge_description(challenge_t* node, const Rectangle& rec, int is_detailed);
static void draw_challenge_top(challenge_t* node, const Rectangle& rec, int is_detailed);
static void draw_challenge_value_bar(challenge_t* node, const Rectangle& rec);
static void draw_challenges_category_points();
static void draw_current_challenge();
static int  draw_text_in_rec(const char* text, const Rectangle& rec);
static void destroy();

static std::string tier_to_next_tier(const std::string& tier);
static Color tier_to_color(const std::string& tier);

static void build_challenges(const nlohmann::json& global_challenges, const nlohmann::json& account_challenges);
static challenge_t* find_id(challenge_t* cur, int id);
static int find_parent_id(int id);

static bool is_within(const Vector2& p, const Rectangle& rec) {
    return rec.x <= p.x && p.x <= rec.x + rec.width && rec.y <= p.y && p.y <= rec.y + rec.height;
}

static challenge_t* find_id(challenge_t* cur, int id) {
    if (cur->id == id) {
        return cur;
    }

    for (challenge_t* child : cur->children) {
        if (child->id == id) {
            return child;
        }
    }

    return 0;
}

static int find_parent_id(int id) {
    if (id <= 0) {
        return -1;
    }

    int divisor = 100;
    while (id % divisor == 0) {
        divisor *= 10;
    }

    int result = id / divisor * divisor;

    if (result % 100000 == 0) {
        return result / 100000;
    }
    return result;
}

static void build_challenges(const nlohmann::json& global_challenges, const nlohmann::json& account_challenges) {
    challenge_t* root = 0;

    std::unordered_map<int, challenge_t*> unconnected_nodes;

    challenge_t* legacy = new challenge_t();
    legacy->id = 6;
    legacy->leaderboard = false;
    legacy->short_description = "Legacy";
    legacy->description = "Legacy";
    legacy->name = "Legacy";
    legacy->state = "DISABLED";
    legacy->tier = "NONE";
    legacy->next_tier = "NONE";
    legacy->percentile = 0;
    legacy->value = 0;
    legacy->next_value = 0;
    legacy->achieved_time = 0;
    legacy->parent = 0;

    unconnected_nodes.insert({ legacy->id, legacy });

    for (const nlohmann::json& global_challenge : global_challenges) {
        nlohmann::json found_account_challenge;
        int found = 0;
        for (nlohmann::json account_challenge : account_challenges["challenges"]) {
            if (account_challenge["challengeId"] == global_challenge["id"]) {
                found_account_challenge = account_challenge;
                found = 1;
                break ;
            }
        }

        challenge_t* challenge = new challenge_t();
        challenge->parent = 0;
        challenge->id = global_challenge["id"];
        challenge->leaderboard = global_challenge["leaderboard"];
        challenge->short_description = global_challenge["localizedNames"]["en_US"]["shortDescription"];
        challenge->description = global_challenge["localizedNames"]["en_US"]["description"];
        challenge->name = global_challenge["localizedNames"]["en_US"]["name"];
        challenge->state = global_challenge["state"];

        const auto find_next_value = [&global_challenge, &challenge]() {
            double result = 0;

            std::vector<double> values;
            for (const auto& j : global_challenge["thresholds"].items()) {
                values.push_back(j.value());
            }
            assert(!values.empty());
            std::sort(values.begin(), values.end());
            result = challenge->value;
            for (int i = 0; i < values.size(); ++i) {
                if (challenge->value < values[i]) {
                    if (i < values.size() - 1) {
                        result = values[i + 1];
                    }
                }
            }

            return result;
        };
        if (found) {
            challenge->tier = found_account_challenge["level"];
            if (found_account_challenge["percentile"].is_number()) {
                challenge->percentile = found_account_challenge["percentile"];
            } else {
                challenge->percentile = 0;
            }
            if (found_account_challenge["value"].is_number()) {
                challenge->value = found_account_challenge["value"];
            } else {
                challenge->value = 0;
            }
            challenge->next_value = find_next_value();
            if (found_account_challenge["achievedTime"].is_number()) {
                challenge->achieved_time = found_account_challenge["achievedTime"];
            } else {
                challenge->achieved_time = 0;
            }

        } else {
            challenge->tier = "UNRANKED";
            challenge->percentile = 0;
            challenge->value = 0;
            challenge->next_value = find_next_value();
            challenge->achieved_time = 0;
        }
        challenge->next_tier = tier_to_next_tier(challenge->tier);

        unconnected_nodes.insert({ challenge->id, challenge });
        if (challenge->id == 0) {
            root = challenge;
        }
    }

    assert(root);

    for (auto it : unconnected_nodes) {
        challenge_t* node = it.second;
        if (node == root) {
            continue ;
        }
        int id = find_parent_id(node->id);
        auto parent_it = unconnected_nodes.find(id);
        challenge_t* parent = 0;
        if (parent_it == unconnected_nodes.end()) {
            parent = legacy;
        } else {
            parent = parent_it->second;
        }
        node->parent = parent;
        parent->children.push_back(node);
    }

    _.challanges_root = root;
    _.current_challange = root;
}

static int init(int argc, char** argv) {
    std::cout << "League Tracker v" << LEAGUE_TRACKER_VERSION_MAJOR << "." << LEAGUE_TRACKER_VERSION_MINOR << std::endl;

    if (argc != 2) {
        std::cerr << "usage: <tracker_bin> <riot_api_key>" << std::endl;
        return 1;
    }

    _.window_w = 2400;
    _.window_h = 1200;

    memset(&_.game_name_text_box, 0, sizeof(_.game_name_text_box));
    memset(&_.tag_line_text_box, 0, sizeof(_.tag_line_text_box));
    _.is_game_name_text_box_active = false;
    _.is_tag_line_text_box_active = false;

    _.riot.init(argv[1]);
    _.asset_manager.init("http", "127.0.0.1", 8081);
    _.asset_manager.add_asset_loader<asset_data_json_t>(
        [](asset_data_json_t* json, const unsigned char* serialized_data, size_t serialized_data_size) {
            try {
                json->json = nlohmann::json::parse(serialized_data, serialized_data + serialized_data_size);
                return 0;
            } catch (std::exception& e) {
                std::cerr << "CLIENT failed to parse json object" << std::endl;
                return 1;
            }
        },
        [](asset_data_json_t* json) {
            (void) json;
            return 0;
        }
    );

    InitWindow(_.window_w, _.window_h, "League Tracker");

#if defined(PLATFORM_WEB)
#else
    SetTargetFPS(60);
#endif

    _.liberation_mono = LoadFont("assets/LiberationMono-Regular.ttf");
    GuiSetFont(_.liberation_mono);

    return 0;
}

static void update(double dt) {
    (void) dt;
}

Vector2 recs_find_mid_p(const std::vector<Rectangle>& recs) {
    Vector2 result = {
        .x = 0.0f,
        .y = 0.0f
    };

    for (int i = 0; i < recs.size(); ++i) {
        Vector2 rec_mid_p = {
            .x = recs[i].x + recs[i].width / 2.0f,
            .y = recs[i].y + recs[i].height / 2.0f
        };
        result.x = (result.x * i + rec_mid_p.x) / (i + 1);
        result.y = (result.y * i + rec_mid_p.y) / (i + 1);
    }

    return result;
}

void recs_offset(std::vector<Rectangle>& recs, const Vector2& p) {
    for (Rectangle& rec : recs) {
        rec.x += p.x;
        rec.y += p.y;
    }
}

static void draw() {
    BeginDrawing();
    ClearBackground(BLACK);
    
    if (!_.challanges_root) {
        const char* label_text_game_name = "Game name:";
        const char* label_text_tag_line  = "Tag line:";
        const int   font_size = 32;
        const float font_spacing = 1.0f;
        const Vector2 label_text_game_name_dims = MeasureTextEx(_.liberation_mono, label_text_game_name, static_cast<float>(font_size), font_spacing);

        const float margin = 5.0f;
        std::vector<Rectangle> gui_recs;
        // [0] == game_name_label_rec
        gui_recs.push_back({
            .x = 0, .y = 0,
            .width = 200.0f, .height = 100.0f
        });
        // [1] == tag_line_label_rec
        gui_recs.push_back({
            .x = gui_recs[0].x, .y = gui_recs[0].y + gui_recs[0].height + margin,
            .width = gui_recs[0].width, .height = gui_recs[0].height
        });
        // [2] == text_input_game_name_rec
        gui_recs.push_back({
            .x = gui_recs[0].x + gui_recs[0].width + margin, .y = gui_recs[0].y,
            .width = 500.0f, .height = gui_recs[0].height
        });
        // [3] == text_input_tag_line_rec
        gui_recs.push_back({
            .x = gui_recs[1].x + gui_recs[1].width + margin, .y = gui_recs[1].y,
            .width = gui_recs[2].width, .height = gui_recs[1].height
        });
        // [4] == text_input_box_button_rec
        gui_recs.push_back({
            .x = gui_recs[2].x + gui_recs[2].width + margin, .y = gui_recs[2].y,
            .width = 100.0f, .height = 100.0f
        });

        const Vector2 gui_recs_mid_p = recs_find_mid_p(gui_recs);
        const Vector2 gui_offset = {
            _.window_w / 2.0f - gui_recs_mid_p.x,
            _.window_h / 2.0f - gui_recs_mid_p.y
        };
        recs_offset(gui_recs, gui_offset);

        int old_font_size = GuiGetStyle(DEFAULT, TEXT_SIZE);
        GuiSetStyle(DEFAULT, TEXT_SIZE, font_size);
        
        GuiLabel(gui_recs[0], label_text_game_name);
        GuiLabel(gui_recs[1], label_text_tag_line);
        
        if (GuiTextBox(gui_recs[2], _.game_name_text_box, ARRAY_SIZE(_.game_name_text_box), _.is_game_name_text_box_active)) {
            _.is_game_name_text_box_active = !_.is_game_name_text_box_active;
        }
        if (GuiTextBox(gui_recs[3], _.tag_line_text_box, ARRAY_SIZE(_.tag_line_text_box), _.is_tag_line_text_box_active)) {
            _.is_tag_line_text_box_active = !_.is_tag_line_text_box_active;
        }

        if (GuiButton(gui_recs[4], "Ok")) {
            std::string game_name = _.game_name_text_box;
            std::string tag_line  = _.tag_line_text_box;
            _.riot.get_puuid_async(
                game_name, tag_line,
                [game_name, tag_line](const std::string& resulting_puuid) {
                    std::cout << "CLIENT successfully got puuid for '" << resulting_puuid << "'" << std::endl;
                    std::cout << "CLIENT successfully got puuid for '" << game_name << "#" << tag_line << "'" << std::endl;
                    _.riot.get_challenges_by_puuid_async(
                        riot_api::REGION_EUW, resulting_puuid,
                        [game_name, tag_line](const nlohmann::json& resulting_challenges_info_for_puuid) {
                            std::cout << "CLIENT successfully got account_challenges for '" << game_name << "#" << tag_line << "'" << std::endl;
                            _.account_challenges = resulting_challenges_info_for_puuid;
                            std::ofstream f("account_challenges.json");
                            f << resulting_challenges_info_for_puuid.dump(4) << std::endl;

                            _.riot.get_challenges_info_async(
                                riot_api::REGION_EUW,
                                [](const nlohmann::json& resulting_challenges_info) {
                                    std::ofstream f("global_challenges.json");
                                    f << resulting_challenges_info.dump(4) << std::endl;

                                    std::vector<int> ids;
                                    for (const nlohmann::json& j : resulting_challenges_info) {
                                        ids.push_back(j["id"]);
                                    }
                                    std::sort(ids.begin(), ids.end());
                                    std::ofstream g("ids.json");
                                    for (int id : ids) {
                                        g << id << " -> " << find_parent_id(id) << std::endl;
                                    }

                                    _.global_challenges = resulting_challenges_info;
                                    build_challenges(_.global_challenges, _.account_challenges);
                                },
                                []() {
                                }
                            );
                        },
                        [game_name, tag_line]() {
                            std::cerr << "CLIENT failed to get account_challenges for '" << game_name << "#" << tag_line << "'" << std::endl;
                        }
                    );
                },
                []() {
                    std::cerr << "CLIENT failed to get puuid" << std::endl;
                }
            );
        }

        GuiSetStyle(DEFAULT, TEXT_SIZE, old_font_size);
    }

    if (_.current_challange) {
        draw_current_challenge();
    }
    // draw_challenges();

    EndDrawing();
}

static void draw_challenges() {
    if (_.account_challenges.empty()) {
        return ;
    }

    draw_challenges_category_points();
}

static std::string tier_to_next_tier(const std::string& tier) {
    if (tier == "NONE") {
        return "UNRANKED";
    } else if (tier == "UNRANKED") {
        return "IRON";
    } else if (tier == "IRON") {
        return "BRONZE";
    } else if (tier == "BRONZE") {
        return "SILVER";
    } else if (tier == "SILVER") {
        return "GOLD";
    } else if (tier == "GOLD") {
        return "PLATINUM";
    } else if (tier == "PLATINUM") {
        return "EMERALD";
    } else if (tier == "EMERALD") {
        return "DIAMOND";
    } else if (tier == "DIAMOND") {
        return "MASTER";
    } else if (tier == "MASTER") {
        return "GRANDMASTER";
    } else if (tier == "GRANDMASTER") {
        return "CHALLENGER";
    } else if (tier == "CHALLENGER") {
        return "CHALLENGER";
    } else {
        assert(0);
    }
}

static Color tier_to_color(const std::string& tier) {
    Color result = BLACK;

    if (tier == "NONE") {
        result = Color{ 255, 20, 20, 20 };
    } else if (tier == "UNRANKED") {
        result = Color{ 255, 66, 51, 48 };
    } else if (tier == "IRON") {
        result = Color{ 255, 66, 51, 48 };
    } else if (tier == "BRONZE") {
        result = Color{ 255, 90, 62, 58 };
    } else if (tier == "SILVER") {
        result = Color{ 255, 111, 128, 138 };
    } else if (tier == "GOLD") {
        result = Color{ 255, 169, 135, 76 };
    } else if (tier == "PLATINUM") {
        result = Color{ 255, 86, 156, 186 };
    } else if (tier == "EMERALD") {
        result = Color{ 255, 53, 127, 103 };
    } else if (tier == "DIAMOND") {
        result = Color{ 255, 78, 146, 188 };
    } else if (tier == "MASTER") {
        result = Color{ 255, 187, 95, 236 };
    } else if (tier == "GRANDMASTER") {
        result = Color{ 255, 159, 43, 40 };
    } else if (tier == "CHALLENGER") {
        result = Color{ 255, 213, 176, 96 };
    } else {
        assert(0);
    }

    return result;
}

static int draw_text_in_rec_helper(const char* text, const Rectangle& rec, size_t n_of_tries_left) {
    // fit rec as best as we can

    const float font_spacing = 1.0f;
    float font_size = 10.0f;
    const float font_size_min = 5.0f;
    Vector2 text_dims = MeasureTextEx(_.liberation_mono, text, font_size, font_spacing);
    while (
        text_dims.x < rec.width &&
        text_dims.y < rec.height
    ) {
        font_size += 1.0f;
        text_dims = MeasureTextEx(_.liberation_mono, text, font_size, font_spacing);
    }
    while (
        font_size_min < font_size &&
        (rec.width < text_dims.x ||
        rec.height < text_dims.y)
    ) {
        font_size -= 1.0f;
        text_dims = MeasureTextEx(_.liberation_mono, text, font_size, font_spacing);
    }
    if (font_size_min < font_size) {
        Vector2 text_p = {
            .x = rec.x + (rec.width - text_dims.x) / 2.0f,
            .y = rec.y + (rec.height - text_dims.y) / 2.0f
        };
        DrawTextEx(_.liberation_mono, text, text_p, font_size, font_spacing, WHITE);

        return 0;
    } else {
        if (!n_of_tries_left) {
            return 1;
        }

        char buffer[16];
        snprintf(buffer, ARRAY_SIZE(buffer), "%.6s..", text);
        return draw_text_in_rec_helper(buffer, rec, n_of_tries_left - 1);
    }
}

static int draw_text_in_rec(const char* text, const Rectangle& rec) {
    return draw_text_in_rec_helper(text, rec, 1);
}

static void draw_challenge_description(challenge_t* node, const Rectangle& rec, int is_detailed) {
    const char* node_description = is_detailed ? node->description.c_str() : node->short_description.c_str();

    draw_text_in_rec(node_description, rec);
}

static void draw_challenge_top(challenge_t* node, const Rectangle& rec, int is_detailed) {
    char buffer[64];
    snprintf(buffer, ARRAY_SIZE(buffer), "top %.2f%%", node->percentile * 100.0f);
    draw_text_in_rec(buffer, rec);
}

static void draw_challenge_value_bar(challenge_t* node, const Rectangle& rec) {
    const float y_margin = rec.height * 0.01f;
    float y_fill = 1.0f;
    const float value_rec_y_fill = y_fill * 0.2f;
    const Rectangle value_rec = {
        .x = rec.x,
        .y = rec.y + y_margin,
        .width = rec.width,
        .height = rec.height * value_rec_y_fill - y_margin
    };
    char buffer[64];
    snprintf(buffer, ARRAY_SIZE(buffer), "value: %.2f, next value: %.2f", node->value, node->next_value);
    draw_text_in_rec(buffer, value_rec);

    const float percentile = node->next_value < node->value ? 0 : node->value / node->next_value;
    Color fill_color = YELLOW;
    Color empty_color = GRAY;

    const float value_bar_rec_y_fill = y_fill;
    Rectangle fill_rec = {
        .x = rec.x,
        .y = value_rec.y + value_rec.height + y_margin,
        .width = rec.width * percentile,
        .height =  rec.height * value_bar_rec_y_fill - y_margin
    };
    Rectangle empty_rec = {
        .x = fill_rec.x + fill_rec.width,
        .y = value_rec.y + value_rec.height + y_margin,
        .width = rec.width - fill_rec.width,
        .height = rec.height * value_bar_rec_y_fill - y_margin
    };
    // draw_text_in_rec;
    DrawRectangleRec(fill_rec, fill_color);
    DrawRectangleRec(empty_rec, empty_color);
}

static void draw_challenge(challenge_t* node, const Rectangle& rec, int is_detailed) {
    DrawRectangleRec(
        rec,
        tier_to_color(node->tier)
    );

    if (is_detailed) {
        const float y_margin = rec.height * 0.01f;
        float y_fill = 1.0f;
        const float description_rec_y_fill = y_fill * 0.5f;
        y_fill -= description_rec_y_fill;
        Rectangle description_rec = {
            .x = rec.x,
            .y = rec.y + y_margin,
            .width = rec.width,
            .height = rec.height * description_rec_y_fill - y_margin
        };
        draw_challenge_description(node, description_rec, is_detailed);

        const float top_rec_y_fill = y_fill * 0.2f;
        y_fill -= top_rec_y_fill;
        Rectangle top_rec = {
            .x = rec.x,
            .y = description_rec.y + description_rec.height + y_margin,
            .width = rec.width,
            .height = rec.height * top_rec_y_fill - y_margin
        };
        draw_challenge_top(node, top_rec, is_detailed);

        const float value_bar_y_fill = y_fill;
        Rectangle value_bar_rec = {
            .x = rec.x,
            .y = top_rec.y + top_rec.height + y_margin,
            .width = rec.width,
            .height = rec.height * value_bar_y_fill - y_margin
        };
        draw_challenge_value_bar(node, value_bar_rec);
    } else {
        draw_challenge_description(node, rec, is_detailed);
    }
}

static void draw_current_challenge() {
    challenge_t* node      = _.current_challange;
    challenge_t* next_node = _.current_challange;

    Rectangle outer_rec = { .x = _.window_w * 0.01f, .y = _.window_h * 0.01f, _.window_w * 0.98f, _.window_h * 0.98f };
    DrawRectangleLinesEx(
        outer_rec,
        1.0f,
        WHITE
    );
    Vector2 mouse_p = GetMousePosition();
    
    if (0 < node->children.size()) {
        struct state {
            Rectangle rec;
            size_t n;
            int depth;
        } states_stack[32];
        int states_stack_top = 0;
        states_stack[states_stack_top++] = (struct state) {
            .rec   = outer_rec,
            .n     = node->children.size(),
            .depth = 0
        };
        size_t child_node_index = 0;
        while (0 < states_stack_top) {
            struct state state = states_stack[--states_stack_top];
            if (1 < state.n) {
                Rectangle rec_left  = state.rec;
                Rectangle rec_right = state.rec;
                size_t n_left = state.n >> 1;
                size_t n_right = state.n - n_left;
                if (state.depth & 1) {
                    rec_right.x += rec_right.width / 2;
                    rec_right.width /= 2;
                    rec_left.width /= 2;
                } else {
                    rec_right.y += rec_right.height / 2;
                    rec_right.height /= 2;
                    rec_left.height /= 2;
                }

                states_stack[states_stack_top++] = (struct state) {
                    .rec = rec_left,
                    .n = n_left,
                    .depth = state.depth + 1
                };
                assert(states_stack_top < ARRAY_SIZE(states_stack));
                states_stack[states_stack_top++] = (struct state) {
                    .rec = rec_right,
                    .n = n_right,
                    .depth = state.depth + 1
                };
            } else {
                assert(child_node_index < node->children.size());
                challenge_t* child_node = node->children[child_node_index++];
                Rectangle child_node_rec = {
                    .x = state.rec.x + state.rec.width * 0.2f,
                    .y = state.rec.y + state.rec.height * 0.2f,
                    .width = state.rec.width * 0.6f,
                    .height = state.rec.height * 0.6f
                };

                draw_challenge(child_node, child_node_rec, 0);

                if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && is_within(mouse_p, child_node_rec)) {
                    next_node = child_node;
                }
            }
        }
    } else {
        draw_challenge(node, outer_rec, 1);
    }

    if (node != next_node) {
        node = next_node;
    } else if (IsMouseButtonPressed(MOUSE_RIGHT_BUTTON) && node->parent) {
        node = node->parent;
    }

    _.current_challange = node;
}

static void draw_challenges_category_points() {
    if (_.account_challenges.empty()) {
        return ;
    }

    const nlohmann::json& collection  = _.account_challenges["categoryPoints"]["COLLECTION"];
    const nlohmann::json& expertise   = _.account_challenges["categoryPoints"]["EXPERTISE"];
    const nlohmann::json& imagination = _.account_challenges["categoryPoints"]["IMAGINATION"];
    const nlohmann::json& teamwork    = _.account_challenges["categoryPoints"]["TEAMWORK"];
    const nlohmann::json& veterancy   = _.account_challenges["categoryPoints"]["VETERANCY"];

    Vector2 margin = {
        .x = 20,
        .y = 20
    };

    float columns[5] = {
        margin.x,
        margin.x + _.window_w * 0.2f,
        margin.x + _.window_w * 0.4f,
        margin.x + _.window_w * 0.6f,
        margin.x + _.window_w * 0.8f
    };
    float rows[6] = {
        margin.y,
        margin.y + _.window_h * 0.1f,
        margin.y + _.window_h * 0.25f,
        margin.y + _.window_h * 0.4f,
        margin.y + _.window_h * 0.6f,
        margin.y + _.window_h * 0.8f
    };

    const float font_size = 36;
    const float font_spacing = 1;
    DrawTextEx(_.liberation_mono, "Collection",  (Vector2){ columns[0], rows[1] }, font_size, font_spacing, WHITE);
    DrawTextEx(_.liberation_mono, "Expertise",   (Vector2){ columns[0], rows[2] }, font_size, font_spacing, WHITE);
    DrawTextEx(_.liberation_mono, "Imagination", (Vector2){ columns[0], rows[3] }, font_size, font_spacing, WHITE);
    DrawTextEx(_.liberation_mono, "Teamwork",    (Vector2){ columns[0], rows[4] }, font_size, font_spacing, WHITE);
    DrawTextEx(_.liberation_mono, "Veterancy",   (Vector2){ columns[0], rows[5] }, font_size, font_spacing, WHITE);

    DrawTextEx(_.liberation_mono, "Current",    (Vector2){ columns[1], rows[0] }, font_size, font_spacing, WHITE);
    DrawTextEx(_.liberation_mono, "Level",      (Vector2){ columns[2], rows[0] }, font_size, font_spacing, WHITE);
    DrawTextEx(_.liberation_mono, "Max",        (Vector2){ columns[3], rows[0] }, font_size, font_spacing, WHITE);
    DrawTextEx(_.liberation_mono, "Percentile", (Vector2){ columns[4], rows[0] }, font_size, font_spacing, WHITE);

    DrawTextEx(_.liberation_mono, TextFormat("%d", static_cast<int>(collection["current"])),                  (Vector2){ columns[1], rows[1] }, font_size, font_spacing, WHITE);
    DrawTextEx(_.liberation_mono, static_cast<std::string>(collection["level"]).c_str(),                      (Vector2){ columns[2], rows[1] }, font_size, font_spacing, WHITE);
    DrawTextEx(_.liberation_mono, TextFormat("%d", static_cast<int>(collection["max"])),                      (Vector2){ columns[3], rows[1] }, font_size, font_spacing, WHITE);
    DrawTextEx(_.liberation_mono, TextFormat("%.2lf", 100.0 * static_cast<double>(collection["percentile"])), (Vector2){ columns[4], rows[1] }, font_size, font_spacing, WHITE);

    DrawTextEx(_.liberation_mono, TextFormat("%d", static_cast<int>(expertise["current"])),                  (Vector2){ columns[1], rows[2] }, font_size, font_spacing, WHITE);
    DrawTextEx(_.liberation_mono, static_cast<std::string>(expertise["level"]).c_str(),                      (Vector2){ columns[2], rows[2] }, font_size, font_spacing, WHITE);
    DrawTextEx(_.liberation_mono, TextFormat("%d", static_cast<int>(expertise["max"])),                      (Vector2){ columns[3], rows[2] }, font_size, font_spacing, WHITE);
    DrawTextEx(_.liberation_mono, TextFormat("%.2lf", 100.0 * static_cast<double>(expertise["percentile"])), (Vector2){ columns[4], rows[2] }, font_size, font_spacing, WHITE);

    DrawTextEx(_.liberation_mono, TextFormat("%d", static_cast<int>(imagination["current"])),                  (Vector2){ columns[1], rows[3] }, font_size, font_spacing, WHITE);
    DrawTextEx(_.liberation_mono, static_cast<std::string>(imagination["level"]).c_str(),                      (Vector2){ columns[2], rows[3] }, font_size, font_spacing, WHITE);
    DrawTextEx(_.liberation_mono, TextFormat("%d", static_cast<int>(imagination["max"])),                      (Vector2){ columns[3], rows[3] }, font_size, font_spacing, WHITE);
    DrawTextEx(_.liberation_mono, TextFormat("%.2lf", 100.0 * static_cast<double>(imagination["percentile"])), (Vector2){ columns[4], rows[3] }, font_size, font_spacing, WHITE);

    DrawTextEx(_.liberation_mono, TextFormat("%d", static_cast<int>(teamwork["current"])),                  (Vector2){ columns[1], rows[4] }, font_size, font_spacing, WHITE);
    DrawTextEx(_.liberation_mono, static_cast<std::string>(teamwork["level"]).c_str(),                      (Vector2){ columns[2], rows[4] }, font_size, font_spacing, WHITE);
    DrawTextEx(_.liberation_mono, TextFormat("%d", static_cast<int>(teamwork["max"])),                      (Vector2){ columns[3], rows[4] }, font_size, font_spacing, WHITE);
    DrawTextEx(_.liberation_mono, TextFormat("%.2lf", 100.0 * static_cast<double>(teamwork["percentile"])), (Vector2){ columns[4], rows[4] }, font_size, font_spacing, WHITE);

    DrawTextEx(_.liberation_mono, TextFormat("%d", static_cast<int>(veterancy["current"])),                  (Vector2){ columns[1], rows[5] }, font_size, font_spacing, WHITE);
    DrawTextEx(_.liberation_mono, static_cast<std::string>(veterancy["level"]).c_str(),                      (Vector2){ columns[2], rows[5] }, font_size, font_spacing, WHITE);
    DrawTextEx(_.liberation_mono, TextFormat("%d", static_cast<int>(veterancy["max"])),                      (Vector2){ columns[3], rows[5] }, font_size, font_spacing, WHITE);
    DrawTextEx(_.liberation_mono, TextFormat("%.2lf", 100.0 * static_cast<double>(veterancy["percentile"])), (Vector2){ columns[4], rows[5] }, font_size, font_spacing, WHITE);
}

#if defined(PLATFORM_WEB)
static void update_and_draw() {
    update(GetFrameTime());
    draw();
}
#endif

static void destroy() {
    CloseWindow();
}

int main(int argc, char** argv) {
    if (init(argc, argv)) {
        return 1;
    }

#if defined(PLATFORM_WEB)
    emscripten_set_main_loop(&update_and_draw, 0, 1);
#else
    double prev = GetTime();
    while (!WindowShouldClose()) {
        double cur = GetTime();
        double dt = cur - prev;
        prev = cur;

        update(dt);
        draw();
    }
#endif

    destroy();

    return 0;
}
