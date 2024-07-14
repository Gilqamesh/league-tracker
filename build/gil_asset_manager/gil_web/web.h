#ifndef GIL_WEB_API
# define GIL_WEB_API

# define GIL_WEB_VERSION_MAJOR 0
# define GIL_WEB_VERSION_MINOR 1

# include <string>
# include <vector>
# include <functional>

/**
 * Example call:
 * std::string fetched_result;
 * async_fetch__create_async(
 *   "http", "127.0.0.1", 8081, "local.html",
 *   [&fetched_result](const char* serialized_data, size_t serialized_data_size) {
 *     fetched_result.assign(serialized_data, serialized_data + serialized_data_size);
 *     // process fetched result ...
 *   },
 *   []() {
 *     // log failure ...
 *   }
 * )
*/
void get_async(
    const std::string& scheme, const std::string& host_name, size_t port, const std::string& path_name,
    const std::function<void(const unsigned char* serialized_data, size_t serialized_data_size)>& on_success,
    const std::function<void()>& on_failure
);

void get_sync(
    const std::string& scheme, const std::string& host_name, size_t port, const std::string& path_name,
    const std::function<void(const unsigned char* serialized_data, size_t serialized_data_size)>& on_success,
    const std::function<void()>& on_failure
);

#endif // GIL_WEB_API
