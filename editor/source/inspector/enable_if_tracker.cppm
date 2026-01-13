module;

#include <cstdint>
#include <memory>
#include <optional>
#include <string>

export module zeytin.inspector.enable_if;

import zeytin.singleton;

export class EnableIfTracker : public Singleton<EnableIfTracker> {
    friend class Singleton<EnableIfTracker>;
public:
    ~EnableIfTracker();

    std::optional<bool> get_value(uint64_t entity_id,
                                 const std::string& variant_type,
                                 const std::string& key_path,
                                 const std::string& method_name);

    void handle_result(uint64_t entity_id,
                       const std::string& variant_type,
                       const std::string& key_path,
                       bool enabled);

private:
    EnableIfTracker();

    struct Impl;
    std::unique_ptr<Impl> pImpl;
};
