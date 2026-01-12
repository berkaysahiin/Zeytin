module;

#include <string>
#include <vector>
#include <optional>

export module zeytin.windows.property_tracker;

import zeytin.singleton;

export class PropertyTrackerWindow : public Singleton<PropertyTrackerWindow> {
    friend class Singleton<PropertyTrackerWindow>;
public:
    ~PropertyTrackerWindow();

    void render();

private:
    PropertyTrackerWindow();

    bool m_engine_running = false;
};
