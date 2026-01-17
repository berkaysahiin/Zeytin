module;

#include <random>
#include <string_view>

module zeytin.common.guid;

namespace {
    uint64_t fnv1a_hash(std::string_view key, uint64_t attempt) {
        constexpr uint64_t offset_basis = 14695981039346656037ull;
        constexpr uint64_t prime = 1099511628211ull;

        uint64_t hash = offset_basis;
        for (const char ch : key) {
            hash ^= static_cast<uint64_t>(ch);
            hash *= prime;
        }

        hash ^= attempt;
        hash *= prime;
        return hash;
    }
}

uint64_t generate_unique_id() {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<uint64_t> dis;

    return dis(gen);
}

uint64_t generate_stable_id(const std::string_view key, const uint64_t attempt) {
    return fnv1a_hash(key, attempt);
}
