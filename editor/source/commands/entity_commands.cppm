module;

#include <string>
#include <optional>
#include <memory>

export module zeytin.command.entity;
import zeytin.command;

export class AddEntityCommand : public Command {
public:
    AddEntityCommand(std::string entity_name);
    ~AddEntityCommand();
    
    void execute() override;
    void undo() override;
    std::optional<std::string> get_description() const override;

private:
    struct Impl;
    std::unique_ptr<Impl> pImpl;
};
