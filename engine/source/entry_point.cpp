import zeytin.application;

int main() {
    Application app{};

    while(!app.should_shutdown()) {
        app.run_frame();
    }

    app.shutdown();
}
