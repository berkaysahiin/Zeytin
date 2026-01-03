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
    
    bool can_undo() const;
    bool can_redo() const;
    
    std::optional<std::string> get_undo_description() const;
    std::optional<std::string> get_redo_description() const;
    
    void clear();

private:
    CommandManager();
    
    struct Impl;
    std::unique_ptr<Impl> pImpl;
};
