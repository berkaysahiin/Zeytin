module;

#include <string_view.h>
#include <memory>

export module zeytin.manipulator.scale;
import zeytin.manipulator;

export class ScaleManipulator : public IManipulator {
public:
    ScaleManipulator();
    ~ScaleManipulator() override;

    void update(Context& ctx) override;
    bool is_active() const override;
    void reset() override;
    std::string_view get_name() const override;

private:
    struct Impl;
    std::unique_ptr<Impl> m_impl;
};
