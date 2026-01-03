module;

#include <string>
#include <cstdint>
#include <optional>
#include <variant>
#include <memory>

export module zeytin.command.property;
import zeytin.command;

export struct PropertyLocation {
    uint64_t entity_id;
    std::string variant_type;
    std::string key_path;
};

export using PropertyValue = std::variant<int, float, bool, std::string>;

export class PropertyChangeCommand : public Command {
public:
    PropertyChangeCommand(
        PropertyLocation location,
        PropertyValue old_value,
        PropertyValue new_value
    );
    
    ~PropertyChangeCommand();
    
    virtual void execute() override;
    virtual void undo() override;
    virtual std::optional<std::string> get_description() const override;

private:
    struct Impl;
    std::unique_ptr<Impl> pImpl;
};
