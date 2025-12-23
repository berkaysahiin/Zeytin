#include "application/application.h"

int main() {
    Application app{};

    while(!app.should_shutdown()) {
        app.run_frame();
    }

    app.shutdown();
}
