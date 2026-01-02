module;

#include <iostream>
#include <chrono>
#include <thread>

#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "zmq/zmq.hpp"

module zeytin.editor.communication;
import zeytin.editor.event;
import zeytin.logger;

EditorCommunication::EditorCommunication()
    : m_running(false)
    , m_initialized(false)
    , m_context(1)
    , m_publisher(m_context, zmq::socket_type::pub)
    , m_subscriber(m_context, zmq::socket_type::sub) {

    initialize();
    start_connection_attempts();

    EditorEventBus::get().subscribe<std::string>(EditorEvent::SyncEditor, [this](std::string json) {
            send_message(json);
    });

    EditorEventBus::get().subscribe<const std::string&>(EditorEvent::LogToEditor, [this](const auto& json) {
            send_message(json);
    });
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
    rapidjson::Document msg;
    msg.SetObject();

    msg.AddMember("type", "engine_started", msg.GetAllocator());

    rapidjson::StringBuffer buffer;
    rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
    msg.Accept(writer);

    send_message(buffer.GetString());
}

void EditorCommunication::send_shutdown_message() {
    rapidjson::Document msg;
    msg.SetObject();

    msg.AddMember("type", "engine_shutdown", msg.GetAllocator());

    rapidjson::StringBuffer buffer;
    rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
    msg.Accept(writer);

    send_message(buffer.GetString());
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
        //log_warning() << "EditorCommunication not initialized" << std::endl;
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
        
        const std::string& type = doc["type"].GetString();
        
        if (type == "entity_property_changed") {
            EditorEventBus::get().publish<const rapidjson::Document&>(EditorEvent::EntityPropertyChanged, doc);
        }
        else if (type == "entity_variant_added") {
            EditorEventBus::get().publish<const rapidjson::Document&>(EditorEvent::EntityVariantAdded, doc);
        }
        else if (type == "entity_variant_removed") {
            EditorEventBus::get().publish<const rapidjson::Document&>(EditorEvent::EntityVariantRemoved, doc);
        }
        else if (type == "entity_removed") {
            EditorEventBus::get().publish<const rapidjson::Document&>(EditorEvent::EntityRemoved, doc);
        }
        else if (type == "enter_play_mode") {
            bool is_paused = doc["is_paused"].GetBool();
            EditorEventBus::get().publish<bool>(EditorEvent::EnterPlayMode, is_paused);
        }
        else if (type == "exit_play_mode") {
            EditorEventBus::get().publish<bool>(EditorEvent::ExitPlayMode, false);
        }
        else if (type == "pause_play_mode") {
            EditorEventBus::get().publish<bool>(EditorEvent::PausePlayMode, true);
        }
        else if (type == "unpause_play_mode") {
            EditorEventBus::get().publish<bool>(EditorEvent::UnPausePlayMode, true);
        }
        else if (type == "engine_start_confirmed") {
            EditorEventBus::get().publish<bool>(EditorEvent::EngineStartConfirmed, true);
        }
        else if (type == "scene") {
			log_info("Scene has been received by the engine");
            std::cout << "Scene is received" << std::endl;
			std::cout << msg << std::endl;
            EditorEventBus::get().publish<const std::string&>(EditorEvent::Scene, msg);
        }
        else if(type == "die") {
            EditorEventBus::get().publish<bool>(EditorEvent::Die, true);
        }
        else if(type == "window_state") {
            EditorEventBus::get().publish<const rapidjson::Document&>(EditorEvent::WindowStateChanged, doc);
        }
        else {
            //log_warning() << "Unknown message type received from editor" << std::endl;
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
            //log_error() << "Error receiving message: " << e.what() << std::endl;
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
    }
}
