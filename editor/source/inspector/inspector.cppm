module;

#include <vector>
#include <memory>

export module zeytin.inspector;

export class Inspector final {
public:
    Inspector();
    ~Inspector();
    
    void render();
private:
    struct Impl;
    std::unique_ptr<Impl> pImpl;
};
