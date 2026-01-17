module;

#include <cstdint>
#include <string_view>

export module zeytin.common.guid;

export {
    uint64_t generate_unique_id();
    uint64_t generate_stable_id(std::string_view key, uint64_t attempt = 0);
}
