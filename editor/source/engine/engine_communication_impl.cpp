module;

#include "rapidjson/document.h"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/writer.h"

#include "zmq/zmq.hpp"

#include <chrono>
#include <thread>
#include <queue>

module zeytin.engine.communication;
import zeytin.engine.event;
import zeytin.engine.controls;
import zeytin.logger;
import zeytin.selection;
import zeytin.entity.document;
import zeytin.command.property;
import zeytin.command.batch_property;
import zeytin.command.manager;

import zeytin.common.message;
import zeytin.common.message_registry;
import zeytin.common.message.engine_to_editor.batch_property_change_command;
import zeytin.common.message.engine_to_editor.engine_shutdown;
import zeytin.common.message.engine_to_editor.engine_started;
import zeytin.common.message.engine_to_editor.entity_selected;
import zeytin.common.message.engine_to_editor.log_message;
import zeytin.common.message.engine_to_editor.property_change_command;
import zeytin.common.message.engine_to_editor.scene;

void EngineCommunication::register_message_handlers() {
    auto& registry = MessageRegistry::get();

    registry.register_handler("scene", [](const rapidjson::Document& doc, const std::string&) {
        SceneMessage message;
        if (message.from_json(doc)) {
            EngineEventBus::get().publish<std::string>(EngineEvent::SyncEditor, message.scene_json);
        }
    });
    registry.register_handler(EngineStartedMessage{}.get_type(), [this](const rapidjson::Document& doc, const std::string&) {
        EngineStartedMessage message;
        if (message.from_json(doc)) {
            send_simple_message("engine_start_confirmed");
            EngineEventBus::get().publish<bool>(EngineEvent::EngineStarted, true);
        }
    });
    registry.register_handler(EngineShutdownMessage{}.get_type(), [](const rapidjson::Document& doc, const std::string&) {
        EngineShutdownMessage message;
        if (message.from_json(doc)) {
            EngineEventBus::get().publish<bool>(EngineEvent::EngineStopped, true);
        }
    });
    registry.register_handler(EntitySelectedMessage{}.get_type(), [](const rapidjson::Document& doc, const std::string&) {
        EntitySelectedMessage message;
        if (message.from_json(doc)) {
            SelectionManager::get().select_entity(message.id);
        }
    });
    registry.register_handler("property_change_command", [](const rapidjson::Document& doc, const std::string&) {
        PropertyChangeCommandMessage message;
        if (message.from_json(doc)) {
            PropertyLocation location;
            location.entity_id = message.entity_id;
            location.variant_type = message.variant_type;
            location.key_path = message.key_path;

            PropertyValue old_value = message.old_value;
            PropertyValue new_value = message.new_value;

            auto command = std::make_unique<PropertyChangeCommand>(location, old_value, new_value);
            CommandManager::get().execute_command(std::move(command));
        }
    });
    registry.register_handler("batch_property_change_command", [](const rapidjson::Document& doc, const std::string&) {
        BatchPropertyChangeCommandMessage message;
        if (message.from_json(doc)) {
            std::vector<PropertyChange> changes;
            changes.reserve(message.changes.size());

            for (const auto& change : message.changes) {
                PropertyChange pc;
                pc.key_path = change.key_path;
                pc.old_value = change.old_value;
                pc.new_value = change.new_value;
                changes.push_back(pc);
            }

            auto command = std::make_unique<BatchPropertyChangeCommand>(message.entity_id, message.variant_type, changes);
            CommandManager::get().execute_command(std::move(command));
        }
    });
    registry.register_handler("log_message", [](const rapidjson::Document& doc, const std::string&) {
        LogMessage message;
        if (message.from_json(doc)) {
            if (message.level == "INFO") {
                log_info("[ENGINE] {}", message.message);
            } else if (message.level == "TRACE") {
                log_trace("[ENGINE] {}", message.message);
            } else if (message.level == "WARNING") {
                log_warning("[ENGINE] {}", message.message);
            } else if (message.level == "ERROR") {
                log_error("[ENGINE] {}", message.message);
            }
        }
    });
}

EngineCommunication::EngineCommunication()
    : m_running(false)
    , m_initialized(false)
    , m_context(1)
    , m_publisher(m_context, zmq::socket_type::pub)
    , m_subscriber(m_context, zmq::socket_type::sub)
{
    initialize();
    register_event_handlers();
    register_message_handlers();
}

EngineCommunication::~EngineCommunication() {
    shutdown();
}

void EngineCommunication::register_event_handlers() {
}

bool EngineCommunication::initialize() {
    if (m_initialized)
        return true;
    
    try {
        m_publisher.bind("tcp://*:5555");
        m_subscriber.bind("tcp://*:5556");
        m_subscriber.set(zmq::sockopt::subscribe, "");
        
        m_running = true;
        m_receive_thread = std::thread(&EngineCommunication::receive_messages, this);
        m_event_thread = std::thread(&EngineCommunication::event_processing_loop, this);
        
        m_initialized = true;
        return true;
    }
    catch (const zmq::error_t& e) {
        //log_error() << "ZeroMQ error: " << e.what() << std::endl;
        return false;
    }
}

void EngineCommunication::event_processing_loop() {
    while (m_running) {
        raise_events();
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
}

void EngineCommunication::raise_events() {
    std::queue<std::string> messages;

    {
        std::lock_guard<std::mutex> lock(m_queue_mutex);
        messages.swap(m_message_queue);
    }

    while (!messages.empty()) {
        const std::string& msg = messages.front();

        if(msg.empty()) {
            continue;
        }

        rapidjson::Document doc;
        doc.Parse(msg.c_str());

        if (doc.HasParseError()) {
            //log_error() << "Failed to parse message: " << msg.substr(0, 100)
                     // << (msg.length() > 100 ? "..." : "") << std::endl;
            //log_error() << "Parse error code: " << doc.GetParseError()
                    //  << " at offset " << doc.GetErrorOffset() << std::endl;
            messages.pop();
            continue;
        }

        if (!doc.IsObject()) {
            // log_error() << "Message is not a valid JSON object" << std::endl;
            messages.pop();
            continue;
        }

        if (!doc.HasMember("type") || !doc["type"].IsString()) {
            // log_error() << "Message missing 'type' field or not a string" << std::endl;
            messages.pop();
            continue;
        }

        if (!MessageRegistry::get().dispatch(doc, msg)) {
            //log_warning() << "Unknown message received from engine" << std::endl;
        }

        messages.pop();
    }
}

void EngineCommunication::shutdown() {
    if (!m_initialized)
        return;
    
    m_running = false;
    
    if (m_receive_thread.joinable()) {
        m_receive_thread.join();
    }

    if (m_event_thread.joinable()) {
        m_event_thread.join();
    }
    
    m_initialized = false;
}

bool EngineCommunication::send_message(const std::string& json) {
    if (!m_initialized) {
        log_error("EngineCommunication not initialized");
        return false;
    }

    if (json.empty()) {
        log_error("Attempted to send empty JSON message");
        return false;
    }

    zmq::message_t message(json.size());
    if (message.size() != json.size()) {
        log_error("Failed to allocate message of size {}", json.size());
        return false;
    }

    memcpy(message.data(), json.data(), json.size());

    auto result = m_publisher.send(message, zmq::send_flags::none);
    return result.has_value();
}

bool EngineCommunication::send_simple_message(const std::string& type,
                                             const std::string& key,
                                             bool value) {
    rapidjson::Document msg;
    msg.SetObject();
    auto& allocator = msg.GetAllocator();

    if(!type.empty()) {
        rapidjson::Value type_value(type.c_str(), allocator);
        msg.AddMember("type", type_value, allocator);
    }
    else {
        log_error("Empty type is not allowed for send_simple_message");
        return false;
    }

    if (!key.empty()) {
        rapidjson::Value keyValue(key.c_str(), msg.GetAllocator());
        rapidjson::Value boolValue(value);
        msg.AddMember(keyValue, boolValue, msg.GetAllocator());
    }

    rapidjson::StringBuffer buffer;
    rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
    msg.Accept(writer);

    return send_message(buffer.GetString());
}

bool EngineCommunication::is_engine_connected() const {
    return m_initialized;
}

void EngineCommunication::receive_messages() {
    while (m_running) {
        zmq::pollitem_t items[] = {
            { static_cast<void*>(m_subscriber), 0, ZMQ_POLLIN, 0 }
        };

        zmq::poll(items, 1, std::chrono::milliseconds(100));

        if (items[0].revents & ZMQ_POLLIN) {
            zmq::message_t message;
            auto result = m_subscriber.recv(message, zmq::recv_flags::none);

            if (result.has_value() && message.size() > 0) {
                if (message.data() != nullptr) {
                    std::string message_str(static_cast<char*>(message.data()), message.size());

                    if (!message_str.empty()) {
                        std::lock_guard<std::mutex> lock(m_queue_mutex);
                        m_message_queue.push(message_str);
                    } else {
                        log_error("Received empty message");
                    }
                } else {
                    log_error("Received message with null data");
                }
            }
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
}
