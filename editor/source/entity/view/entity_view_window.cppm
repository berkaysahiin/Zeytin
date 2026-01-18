module;

#include <memory>

export module zeytin.entity.view;

export class EntityViewWindow {
public:
    EntityViewWindow();
    ~EntityViewWindow();

    void render();

private:
    struct Impl;
    std::unique_ptr<Impl> pImpl;
};
