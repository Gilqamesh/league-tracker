#include "gil_asset_manager/gil_web/web.cpp"

#include <iostream>
#include <chrono>
#include <thread>
#include <regex>
#include <fstream>

void get_async(
    const std::string& scheme, const std::string& host_name, size_t port, const std::string& path_name,
    const std::function<void(const unsigned char* serialized_data, size_t serialized_data_size)>& on_success,
    const std::function<void()>& on_failure
);

int main() {
    get_async(
        "https", "raw.communitydragon.org", 443, "latest/plugins/rcp-be-lol-game-data/global/default/v1/champions/",
        [](const unsigned char* serialized_data, size_t serialized_data_size) {
            std::string result(serialized_data, serialized_data + serialized_data_size);
            std::regex pattern("<a href=\"[0-9]+\\.json");
            auto words_begin = std::sregex_iterator(result.begin(), result.end(), pattern);
            auto words_end = std::sregex_iterator();

            if (words_begin != words_end) {
                std::cout << "Match found!" << std::endl;
                for (std::sregex_iterator i = words_begin; i != words_end; ++i) {
                    std::smatch match = *i;
                    std::string match_str = match.str();
                    std::string::size_type p = match_str.find_first_of('"');
                    assert(p != std::string::npos);
                    std::string json_file_name = match_str.substr(p + 1);
                    get_async(
                        "https", "raw.communitydragon.org", 443, "latest/plugins/rcp-be-lol-game-data/global/default/v1/champions/" + json_file_name,
                        [&json_file_name](const unsigned char* serialized_data, size_t serialized_data_size) {
                            std::cout << "writing subresult to json_files/" << json_file_name << std::endl;
                            std::ofstream of("json_files/" + json_file_name);
                            of << std::string(serialized_data, serialized_data + serialized_data_size);
                        },
                        []() {
                            std::cout << "subresult not found" << std::endl;
                        }
                    );
                    std::cout << "Found match: " << match_str << std::endl;
                }
            } else {
                std::cout << "No match found." << std::endl;
            }
        },
        []() {
            std::cout << "failed" << std::endl;
        }
    );

    std::this_thread::sleep_for(std::chrono::duration<double, std::ratio<1, 1>>(5));

    return 0;
}
