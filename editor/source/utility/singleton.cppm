export module zeytin.singleton;

export template<typename Derived>
class Singleton {
public:
    static Derived& get() {
        static Derived instance;
        return instance;
    }

    Singleton(const Singleton&) = delete;
    Singleton& operator=(const Singleton&) = delete;

protected:
    Singleton() = default;
    ~Singleton() = default;
};
