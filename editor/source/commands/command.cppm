module;

#include <string>
#include <optional>

export module zeytin.command;

export class Command {
public:
    virtual ~Command() = default;
    
    virtual void execute() = 0;
    virtual void undo() = 0;

    [[nodiscard]] 
	virtual std::optional<std::string> get_description() const { return std::nullopt; }
};
