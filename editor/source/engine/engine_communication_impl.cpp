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

EngineCommunication::EngineCommunication()
    : m_running(false)
    , m_initialized(false)
    , m_context(1)
    , m_publisher(m_context, zmq::socket_type::pub)
    , m_subscriber(m_context, zmq::socket_type::sub) 
{
    initialize();
    register_event_handlers();
}

EngineCommunication::~EngineCommunication() {
    shutdown();
}

void EngineCommunication::register_event_handlers() {
    EngineEventBus::get().subscribe<const std::string&>(EngineEvent::EngineSendScene,
            [this](const auto& msg) {
                send_message(msg);
    });

    EngineEventBus::get().subscribe<const std::string&>(
        EngineEvent::EntityModifiedEditor, 
        [this](const std::string& msg) {
            send_message(msg);
        }
    );

    EngineEventBus::get().subscribe<bool>(
        EngineEvent::EnterPlayMode, 
        [this](bool paused) {
            send_simple_message("enter_play_mode", "is_paused", paused);
        }
    );

    EngineEventBus::get().subscribe<bool>(
        EngineEvent::ExitPlayMode, 
        [this](bool) {
            send_simple_message("exit_play_mode");
        }
    );

    EngineEventBus::get().subscribe<bool>(
        EngineEvent::PausePlayMode, 
        [this](bool) {
            send_simple_message("pause_play_mode");
        }
    );

    EngineEventBus::get().subscribe<bool>(
        EngineEvent::UnPausePlayMode, 
        [this](bool) {
            send_simple_message("unpause_play_mode");
        }
    );

    EngineEventBus::get().subscribe<bool>(
        EngineEvent::KillEngine, 
        [this](bool) {
            send_simple_message("die");
        }
    );

    EngineEventBus::get().subscribe<const std::string&>(
        EngineEvent::WindowStateChanged,
        [this](const std::string& message) {
            send_message(message);
        }
    );
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

        const std::string type = doc["type"].GetString();

        if(type.empty()) {
            continue;
        }

        if (type == "scene") {
            EngineEventBus::get().publish<std::string>(EngineEvent::SyncEditor, msg);
        }
        else if (type == "engine_started") {
            send_simple_message("engine_start_confirmed");
            EngineEventBus::get().publish<bool>(EngineEvent::EngineStarted, true);
        }
        else if (type == "engine_shutdown") {
            // log_info() << "Engine shutdown" << std::endl;
            EngineEventBus::get().publish<bool>(EngineEvent::EngineStopped, true);
        }
        else if(type == "log_message") {
            if(doc.HasMember("level") && doc.HasMember("message")) {
                assert(doc["level"].IsString());
                assert(doc["message"].IsString());

                std::string level = doc["level"].GetString();
                std::string msg = doc["message"].GetString();

                if(level == "INFO") {
                   // Logger::get().info() << "[ENGINE] " << msg;
                }
                else if(level == "TRACE") {
                    //Logger::get().trace() << "[ENGINE] " << msg;
                }
                else if(level == "WARNING") {
                    //Logger::get().warning() << "[ENGINE] " << msg;
                }
                else if(level == "ERROR") {
                    //Logger::get().error() << "[ENGINE] " << msg;
                }
            }
        }
        else {
            //log_warning() << "Unknown message received from engine: " << type << std::endl;
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
        //log_error() << "EngineCommunication not initialized" << std::endl;
        return false;
    }

    if (json.empty()) {
        //log_error() << "Attempted to send empty JSON message" << std::endl;
        return false;
    }

    zmq::message_t message(json.size());
    if (message.size() != json.size()) {
        //log_error() << "Failed to allocate message of size " << json.size() << std::endl;
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
        //log_error() << "Empty type is not allowed for send_simple_message" << std::endl;
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
                        // log_error() << "Received empty message" << std::endl;
                    }
                } else {
                    // log_error() << "Received message with null data" << std::endl;
                }
            }
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
}
