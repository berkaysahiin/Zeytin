module;

#include <type_traits>
#include <utility>

export module zeytin.editor.message;

#ifdef EDITOR_MODE

export import zeytin.common.message;
export import zeytin.common.message.engine_to_editor.entity_selected;
import zeytin.editor.communication;

export void send_message_to_editor(const IMessage& message) {
    EditorCommunication::get().send_message(message.as_json());
}

export template<typename Message, typename... Args>
requires std::is_base_of_v<IMessage, Message>
void send_message_to_editor(Args&&... args) {
    send_message_to_editor(Message{std::forward<Args>(args)...});
}

#endif
