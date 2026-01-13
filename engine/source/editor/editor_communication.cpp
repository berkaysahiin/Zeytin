module;

#include <iostream>
#include <chrono>
#include <thread>

#include "rapidjson/document.h"
#include "zmq/zmq.hpp"

module zeytin.editor.communication;
import zeytin.common.message.engine_to_editor.engine_started;
import zeytin.common.message.engine_to_editor.engine_shutdown;
import zeytin.common.message.editor_to_engine.tracked_property_request;
import zeytin.common.message.editor_to_engine.enable_if_request;
import zeytin.common.message_registry;
import zeytin.editor.event;
import zeytin.logger;

namespace {
    void register_message_handlers() {
        auto& registry = MessageRegistry::get();

        registry.register_handler("entity_property_changed", [](const rapidjson::Document& doc, const std::string&) {
            EditorEventBus::get().publish<const rapidjson::Document&>(EditorEvent::EntityPropertyChanged, doc);
        });
        registry.register_handler("entity_variant_added", [](const rapidjson::Document& doc, const std::string&) {
            EditorEventBus::get().publish<const rapidjson::Document&>(EditorEvent::EntityVariantAdded, doc);
        });
        registry.register_handler("entity_variant_removed", [](const rapidjson::Document& doc, const std::string&) {
            EditorEventBus::get().publish<const rapidjson::Document&>(EditorEvent::EntityVariantRemoved, doc);
        });
        registry.register_handler("entity_added", [](const rapidjson::Document& doc, const std::string&) {
            EditorEventBus::get().publish<const rapidjson::Document&>(EditorEvent::EntityAdded, doc);
        });
        registry.register_handler("entity_removed", [](const rapidjson::Document& doc, const std::string&) {
            EditorEventBus::get().publish<const rapidjson::Document&>(EditorEvent::EntityRemoved, doc);
        });
        registry.register_handler("enter_play_mode", [](const rapidjson::Document& doc, const std::string&) {
            bool is_paused = doc.HasMember("is_paused") && doc["is_paused"].IsBool() ? doc["is_paused"].GetBool() : false;
            EditorEventBus::get().publish<bool>(EditorEvent::EnterPlayMode, is_paused);
        });
        registry.register_handler("exit_play_mode", [](const rapidjson::Document&, const std::string&) {
            EditorEventBus::get().publish<bool>(EditorEvent::ExitPlayMode, false);
        });
        registry.register_handler("pause_play_mode", [](const rapidjson::Document&, const std::string&) {
            EditorEventBus::get().publish<bool>(EditorEvent::PausePlayMode, true);
        });
        registry.register_handler("unpause_play_mode", [](const rapidjson::Document&, const std::string&) {
            EditorEventBus::get().publish<bool>(EditorEvent::UnPausePlayMode, true);
        });
        registry.register_handler("engine_start_confirmed", [](const rapidjson::Document&, const std::string&) {
            EditorEventBus::get().publish<bool>(EditorEvent::EngineStartConfirmed, true);
        });
        registry.register_handler("scene", [](const rapidjson::Document&, const std::string& raw) {
            log_info("Scene has been received by the engine");
            EditorEventBus::get().publish<const std::string&>(EditorEvent::Scene, raw);
        });
        registry.register_handler("die", [](const rapidjson::Document&, const std::string&) {
            EditorEventBus::get().publish<bool>(EditorEvent::Die, true);
        });
        registry.register_handler("window_state", [](const rapidjson::Document& doc, const std::string&) {
            EditorEventBus::get().publish<const rapidjson::Document&>(EditorEvent::WindowStateChanged, doc);
        });
        registry.register_handler("entity_selected", [](const rapidjson::Document& doc, const std::string&) {
            EditorEventBus::get().publish<const rapidjson::Document&>(EditorEvent::EntitySelected, doc);
        });
        registry.register_handler(EditorTrackedPropertyRequestMessage{}.get_type(), [](const rapidjson::Document& doc, const std::string&) {
            EditorEventBus::get().publish<const rapidjson::Document&>(EditorEvent::TrackedPropertyRequest, doc);
        });
        registry.register_handler(EnableIfRequestMessage{}.get_type(), [](const rapidjson::Document& doc, const std::string&) {
            EditorEventBus::get().publish<const rapidjson::Document&>(EditorEvent::EnableIfRequest, doc);
        });
    }
}

EditorCommunication::EditorCommunication()
    : m_running(false)
    , m_initialized(false)
    , m_context(1)
    , m_publisher(m_context, zmq::socket_type::pub)
    , m_subscriber(m_context, zmq::socket_type::sub) {

    initialize();
    start_connection_attempts();
    register_message_handlers();
}

EditorCommunication::~EditorCommunication() {
    shutdown();
}

bool EditorCommunication::initialize() {
    if (m_initialized)
        return true;

    try {
        m_subscriber.connect("tcp://localhost:5555");
        m_publisher.connect("tcp://localhost:5556");

        m_subscriber.set(zmq::sockopt::subscribe, "");

        m_running = true;
        m_receive_thread = std::thread(&EditorCommunication::receive_messages, this);

        m_initialized = true;

        return true;
    }
    catch (const zmq::error_t& e) {
        //log_error() << "ZeroMQ error: " << e.what() << std::endl;
        return false;
    }
}

void EditorCommunication::start_connection_attempts() {
    std::thread([this]() {
        EditorEventBus::get().subscribe<bool>(EditorEvent::EngineStartConfirmed,
            [this](bool) {
                m_connection_confirmed = true;
            });

        int attempts = 0;
        const int max_attempts = 30;

        while (m_running && !m_connection_confirmed && attempts < max_attempts) {
            send_started_message();
            std::this_thread::sleep_for(std::chrono::milliseconds(1000));
            attempts++;
        }

        if (m_connection_confirmed) {
            log_info("Connection to editor confirmed!");
        } else if (attempts >= max_attempts) {
            log_error("Failed to connect to editor after {} attempts", max_attempts);
        }
    }).detach();
}

void EditorCommunication::send_started_message() {
    send_message(EngineStartedMessage{}.as_json());
}

void EditorCommunication::send_shutdown_message() {
    send_message(EngineShutdownMessage{}.as_json());
}

void EditorCommunication::shutdown() {
    if (!m_initialized)
        return;
    
    send_shutdown_message();

    m_running = false;
    
    if (m_receive_thread.joinable()) {
        m_receive_thread.join();
    }
    
    m_initialized = false;
}

bool EditorCommunication::send_message(const std::string& message) {
    if (!m_initialized) {
        log_warning("EditorCommunication not initialized");
        return false;
    }
    
    try {
        zmq::message_t zmq_message(message.size());
        memcpy(zmq_message.data(), message.data(), message.size());
        
        auto result = m_publisher.send(zmq_message, zmq::send_flags::none);
        return result.has_value();
    }
    catch (const std::exception& e) {
        //log_warning() << "Failed to send message: " << e.what() << std::endl;
        return false;
    }
}

void EditorCommunication::raise_events() {
    while (!m_message_queue.empty()) {
        const auto& msg = m_message_queue.front();
        rapidjson::Document doc;
        doc.Parse(msg.c_str());
        
        if (doc.HasParseError() || !doc.HasMember("type")) {
            log_warning("Invalid message format received");
            m_message_queue.pop();
            continue;
        }
        
        if (!MessageRegistry::get().dispatch(doc, msg)) {
            log_warning("Unknown message type received from editor");
        }
        
        m_message_queue.pop();
    }
}

void EditorCommunication::receive_messages() {
    while (m_running) {
        try {
            zmq::pollitem_t items[] = {
                { static_cast<void*>(m_subscriber), 0, ZMQ_POLLIN, 0 }
            };
            
            zmq::poll(items, 1, std::chrono::milliseconds(100));
            
            if (items[0].revents & ZMQ_POLLIN) {
                zmq::message_t message;
                auto result = m_subscriber.recv(message, zmq::recv_flags::none);
                
                if (result.has_value()) {
                    std::string message_str(static_cast<char*>(message.data()), message.size());
                    
                    {
                        std::lock_guard<std::mutex> lock(m_queue_mutex);
                        m_message_queue.push(message_str);
                    }
                }
            }
        }
        catch (const zmq::error_t& e) {
            log_error("Error receiving message: {}", e.what());
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
    }
}
