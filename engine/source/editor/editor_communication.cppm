module;

#include <thread>
#include <mutex>
#include <queue>
#include <string>
#include <atomic>
#include "zmq/zmq.hpp"

export module zeytin.editor.communication;
import zeytin.singleton;

export class EditorCommunication : public Singleton<EditorCommunication> {
	friend class Singleton<EditorCommunication>;
public:
    EditorCommunication();
    ~EditorCommunication();

    bool initialize();
    void shutdown();
    bool send_message(const std::string& json);
    void raise_events();

    bool is_connection_confirmed() const { return m_connection_confirmed; }

private:
    void receive_messages();
    void start_connection_attempts();
    void send_started_message();
    void send_shutdown_message();
    
    bool m_running;
    bool m_initialized;

    std::atomic<bool> m_connection_confirmed{false};

    zmq::context_t m_context;
    zmq::socket_t m_publisher;
    zmq::socket_t m_subscriber;
    std::thread m_receive_thread;
    std::mutex m_queue_mutex;
    std::queue<std::string> m_message_queue;
};
