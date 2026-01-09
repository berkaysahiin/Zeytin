module;

#include <string>
#include <cstdint>
#include <optional>
#include <memory>
#include <vector>

export module zeytin.command.batch_property;
import zeytin.command;
import zeytin.command.property;

export struct PropertyChange {
    std::string key_path;           
    PropertyValue old_value;
    PropertyValue new_value;
};

export class BatchPropertyChangeCommand : public Command {
public:
    BatchPropertyChangeCommand(
        uint64_t entity_id,
        std::string variant_type,
        std::vector<PropertyChange> changes
    );
    
    ~BatchPropertyChangeCommand();
    
    virtual void execute() override;
    virtual void undo() override;
    virtual std::optional<std::string> get_description() const override;

private:
    struct Impl;
    std::unique_ptr<Impl> pImpl;
};
