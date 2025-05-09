#include "application/application.h"
#include "game/generated/rttr_registration.h"

int main() {
    Application app{};

    while(!app.should_shutdown()) {
        app.run_frame();
    }

    app.shutdown();
}
