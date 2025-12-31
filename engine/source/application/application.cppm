export module zeytin.application;

export class Application {
public:
    Application();
    ~Application();

    void run_frame();
    void shutdown();
    bool should_shutdown();

private:
    void init_window();
    void init_engine();

	struct Impl;
	Impl *pImpl;
};
