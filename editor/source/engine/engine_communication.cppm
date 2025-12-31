module;

#include <thread>
#include <mutex>
#include <queue>
#include <string>

#include "zmq/zmq.hpp"

export module zeytin.engine.communication;

export class EngineCommunication {
public:
    EngineCommunication();
    ~EngineCommunication();

    bool initialize();
    void shutdown();
    bool send_message(const std::string& json);
    bool is_engine_connected() const;
    void raise_events();

private:
    void register_event_handlers();
    void receive_messages();
    void event_processing_loop();
    bool send_simple_message(const std::string& type, const std::string& key = "", bool value = false);

    bool m_running;
    bool m_initialized;

    zmq::context_t m_context;
    zmq::socket_t m_publisher;
    zmq::socket_t m_subscriber;

    std::thread m_receive_thread;
    std::thread m_event_thread;
    std::mutex m_queue_mutex;
    std::queue<std::string> m_message_queue;
};
