module;

#include <string>
#include <cstdint>
#include <optional>
#include <memory>

export module zeytin.command.component;
import zeytin.command;

export class AddComponentCommand : public Command {
public:
    AddComponentCommand(uint64_t entity_id, const std::string& component_type);
    ~AddComponentCommand();
    
    void execute() override;
    void undo() override;
    std::optional<std::string> get_description() const override;

private:
    struct Impl;
    std::unique_ptr<Impl> pImpl;
};

export class RemoveComponentCommand : public Command {
public:
    RemoveComponentCommand(uint64_t entity_id, const std::string& component_type);
    ~RemoveComponentCommand();
    
    void execute() override;
    void undo() override;
    std::optional<std::string> get_description() const override;

private:
    struct Impl;
    std::unique_ptr<Impl> pImpl;
};
