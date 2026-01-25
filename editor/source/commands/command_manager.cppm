module;

#include <memory>
#include <optional>
#include <string>

export module zeytin.command.manager;
import zeytin.command;
import zeytin.singleton;

export class CommandManager : public Singleton<CommandManager> {
    friend class Singleton<CommandManager>;
public:
    ~CommandManager();
    
    void execute_command(std::unique_ptr<Command> command);
    
    void undo();
    void redo();
    
    [[nodiscard]] 
	bool can_undo() const;

    [[nodiscard]] 
	bool can_redo() const;
    
    [[nodiscard]] 
	std::optional<std::string> get_undo_description() const;

    [[nodiscard]] 
	std::optional<std::string> get_redo_description() const;
    
    void clear();
    
    [[nodiscard]] 
	size_t get_history_size() const;

    [[nodiscard]] 
	size_t get_current_position() const;

    [[nodiscard]] 
	std::optional<std::string> get_command_description( size_t index) const;

private:
    CommandManager();
    
    struct Impl;
    std::unique_ptr<Impl> pImpl;
};
