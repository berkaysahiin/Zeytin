module;

export module zeytin.editor.message;
export import zeytin.common.message;
export import zeytin.common.message.entity_selected;
import zeytin.editor.communication;

export void send_to_editor(const IMessage& message) {
    EditorCommunication::get().send_message(message.as_json());
}
