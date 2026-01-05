module;

#include <vector>
#include <memory>
#include <optional>
#include <string>

module zeytin.command.manager;
import zeytin.logger;
import zeytin.entity.registry;
import zeytin.level;

// The implementation treats undo/redo as navigating through a timeline of executed commands.
//
// - command_history: A vector storing all commands in execution order
// - current_position: An index that acts as a "playhead" showing where we are in the history
//
// 1. Commands before current_position are "done" - they've been executed and can be undone
// 2. Commands at/after current_position are "undone" - they can be redone
// 3. current_position ranges from 0 to command_history.size():
//    - 0 means nothing has been done (can't undo)
//    - size() means everything is done (can't redo)
//
// Example Timeline:
// History: [A, B, C]
// Position: 2
//          [A, B | C]
//           ^done | ^undone

struct CommandManager::Impl {
	// Vector of unique_ptrs to Command objects
    // Each command is owned exclusively by this vector
    std::vector<std::unique_ptr<Command>> command_history;
    size_t current_position = 0;
};

CommandManager::CommandManager() 
    : pImpl(std::make_unique<Impl>()) {
	// Not sure about cleaning command history on level change.
	//auto entity_list_opt = EntityRegistry::get().get_entity_list();
    //if (entity_list_opt.has_value()) {
    //    entity_list_opt->get().add_level_unloading_callback([this](const Level&) {
	//		clear();
    //    });
    //}
}

CommandManager::~CommandManager() = default;

void CommandManager::execute_command(std::unique_ptr<Command> command) {
    // When executing a new command after undoing, we discard the undone commands.
    // This matches standard undo/redo behavior.
    //
    // Example:
    // 1. Do actions A, B, C
    // 2. Undo C (position moves to 2)
    // 3. Do action D
	//
    // History becomes [A, B, D], not [A, B, D, C]
    //
    // Action C is lost because we branched off in a new direction.

	const std::string command_desc = command->get_description().value_or("Unknown command");
	log_trace("Executing command: {}", command_desc);

    if (pImpl->current_position < pImpl->command_history.size()) {
        pImpl->command_history.erase(
            pImpl->command_history.begin() + pImpl->current_position,
            pImpl->command_history.end()
        );
    }
    
    command->execute();
    
    pImpl->command_history.push_back(std::move(command));
    pImpl->current_position = pImpl->command_history.size();
}

void CommandManager::undo() {
    if (!can_undo()) {
        return;
    }
    
    // Move position backwards, then call undo() on the command we just moved past.
    // The command stays in history so it can be redone.
    //
    // Example:
    // Before: History [A, B, C], position = 3
    // After:  History [A, B, C], position = 2, command C is undone
    
    pImpl->current_position--;
    pImpl->command_history[pImpl->current_position]->undo();
}

void CommandManager::redo() {
    if (!can_redo()) {
        return;
    }
    
    // Execute the command at current position, then move forward.
    //
    // Example:
    // Before: History [A, B, C], position = 2 (C was undone)
    // After:  History [A, B, C], position = 3, command C is executed again
    
    pImpl->command_history[pImpl->current_position]->execute();
    pImpl->current_position++;
}

bool CommandManager::can_undo() const {
    return pImpl->current_position > 0;
}

bool CommandManager::can_redo() const {
    return pImpl->current_position < pImpl->command_history.size();
}

std::optional<std::string> CommandManager::get_undo_description() const {
    if (!can_undo()) {
        return std::nullopt;
    }
    return pImpl->command_history[pImpl->current_position - 1]->get_description();
}

std::optional<std::string> CommandManager::get_redo_description() const {
    if (!can_redo()) {
        return std::nullopt;
    }
    return pImpl->command_history[pImpl->current_position]->get_description();
}

void CommandManager::clear() {
    pImpl->command_history.clear();
    pImpl->current_position = 0;
}
