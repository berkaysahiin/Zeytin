module;

#include <functional>
#include <map>
#include <vector>
#include <any>
#include <mutex>
#include <shared_mutex>
#include <algorithm>

export module zeytin.editor.event;

#ifdef EDITOR_MODE

export enum class EditorEvent {
    EngineStartConfirmed,
    Scene,
	EntityAdded,
    EntityRemoved,
    EntityPropertyChanged,
    EntityVariantAdded,
    EntityVariantRemoved,
    EnterPlayMode,
    PausePlayMode,
    UnPausePlayMode,
    ExitPlayMode,
    SyncEditor,
    Die,
    LogToEditor,
    WindowStateChanged,
};

export class EditorEventBus {
public:
    static EditorEventBus& get() {
        static EditorEventBus instance;
        return instance;
    }

    template<typename T>
    void publish(EditorEvent event, const T& data) {
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
    void subscribe(EditorEvent event, std::function<void(const T&)> callback) {
        std::unique_lock<std::shared_mutex> lock(m_mutex);
        m_subscribers[event].push_back(std::any(callback));
    }
    
    template<typename T>
    void unsubscribe(EditorEvent event, const std::function<void(const T&)>* callback_ptr) {
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
    EditorEventBus() = default;
    ~EditorEventBus() = default;
    
    std::map<EditorEvent, std::vector<std::any>> m_subscribers;
    std::shared_mutex m_mutex; 
};

#endif
