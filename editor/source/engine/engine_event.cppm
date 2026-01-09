module;

#include <functional>
#include <map>
#include <vector>
#include <any>
#include <mutex>
#include <shared_mutex>
#include <algorithm>

export module zeytin.engine.event;

export enum class EngineEvent {
    EngineStarted,
    EngineSendScene,
    KillEngine,
    EngineStopped,
    EntityModifiedEditor,
    EntitySelectedEditor,
    EnterPlayMode,
    PausePlayMode,
    UnPausePlayMode,
    ExitPlayMode,
    SyncEditor,
    WindowStateChanged,
};

export class EngineEventBus {
public:
    static EngineEventBus& get() {
        static EngineEventBus instance;
        return instance;
    }

    template<typename T>
    void publish(EngineEvent event, const T& data) {
        std::shared_lock<std::shared_mutex> lock(m_mutex);
        
        auto it = m_subscribers.find(event);
        if (it != m_subscribers.end()) {
            auto callbacks = it->second;
            lock.unlock();  
            
            for (auto& callback : callbacks) {
                if (auto func = std::any_cast<std::function<void(const T&)>>(&callback)) {
                    (*func)(data);
                }
            }
        }
    }

    template<typename T>
    void subscribe(EngineEvent event, std::function<void(const T&)> callback) {
        std::unique_lock<std::shared_mutex> lock(m_mutex);
        m_subscribers[event].push_back(std::any(callback));
    }
    
    template<typename T>
    void unsubscribe(EngineEvent event, const std::function<void(const T&)>* callback_ptr) {
        std::unique_lock<std::shared_mutex> lock(m_mutex);
        
        auto it = m_subscribers.find(event);
        if (it != m_subscribers.end()) {
            auto& callbacks = it->second;
            callbacks.erase(
                std::remove_if(callbacks.begin(), callbacks.end(), 
                    [callback_ptr](const std::any& item) {
                        auto func = std::any_cast<std::function<void(const T&)>>(&item);
                        return func && func->target_type() == callback_ptr->target_type();
                    }),
                callbacks.end()
            );
        }
    }

private:
    EngineEventBus() = default;
    ~EngineEventBus() = default;
    
    std::map<EngineEvent, std::vector<std::any>> m_subscribers;
    std::shared_mutex m_mutex; 
};
