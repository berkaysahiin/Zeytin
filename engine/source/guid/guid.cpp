module;

#include <random>

module zeytin.guid;

uint64_t generate_unique_id() {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<uint64_t> dis;

    return dis(gen);
}
