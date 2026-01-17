module;

#include <vector>
#include <memory>

export module zeytin.inspector;
import zeytin.component.document;

export class Inspector final {
public:
    Inspector(std::vector<ComponentDocument>& variants);
    ~Inspector();
    
    void render();
private:
    struct Impl;
    std::unique_ptr<Impl> pImpl;
};
