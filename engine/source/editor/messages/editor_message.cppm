module;

#include <string>
#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"

export module zeytin.editor.message;
import zeytin.entity;
import zeytin.editor.communication;

export struct IMessage 
{
    virtual ~IMessage() = default;
    virtual std::string get_type() const = 0;
    virtual std::string as_json() const = 0;
};

export struct EntitySelectedMessage : public IMessage
{
    EntityID id;

    explicit EntitySelectedMessage(EntityID entity_id) : id(entity_id) {}

    std::string get_type() const override {
        return "entity_selected_from_engine";
    }

    std::string as_json() const override {
        rapidjson::Document msg;
        msg.SetObject();
        auto& alloc = msg.GetAllocator();

        msg.AddMember("type", rapidjson::Value(get_type().c_str(), alloc), alloc);
        msg.AddMember("entity_id", id, alloc);

        rapidjson::StringBuffer buffer;
        rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
        msg.Accept(writer);

        return buffer.GetString();
    }
};

export void send_to_editor(const IMessage& message) {
    EditorCommunication::get().send_message(message.as_json());
}
