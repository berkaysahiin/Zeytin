module;

#include <memory>
#include <cstdint>

export module zeytin.manipulator.manager;
import zeytin.manipulator;
import zeytin.singleton;

using EntityID = uint64_t;

export class ManipulatorManager : public Singleton<ManipulatorManager> {
    friend class Singleton<ManipulatorManager>;

public:
    void initialize();
    void register_manipulator(const ManipulatorType type, std::unique_ptr<IManipulator> manipulator);
    void set_active(const ManipulatorType type);
    void handle_keyboard_shortcuts();
    void handle_selected(const EntityID selected_entity_id);

    ManipulatorType get_active_type() const;
    bool is_any_active() const;

private:
    ManipulatorManager();
    ~ManipulatorManager();

    struct Impl;
    std::unique_ptr<Impl> m_impl;
};
