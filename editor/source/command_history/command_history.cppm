module;

#include <memory>

export module zeytin.command_history;

export class CommandHistory {
public:
    CommandHistory();
    ~CommandHistory();

    void render();
private:
    struct Impl;
    std::unique_ptr<Impl> pImpl;
};
