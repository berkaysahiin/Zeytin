module;

#include <type_traits>
#include <utility>

export module zeytin.engine.message;

export import zeytin.common.message;
import zeytin.engine.communication;

export void send_message_to_engine(const IMessage& message) {
    EngineCommunication::get().send_message(message.as_json());
}

export template<typename Message, typename... Args>
requires std::is_base_of_v<IMessage, Message>
void send_message_to_engine(Args&&... args) {
    send_message_to_engine(Message{std::forward<Args>(args)...});
}
