#include <charconv>
#include <iostream>
#include <string>
#include <utility>

#include "method.h"
#include "property.h"
#include "reflect.h"
#include "type_builder.h"

struct Vector2 {
    Vector2() = default;

    ZPROPERTY(Category="Math", Desc="X Position");
    int x;

    ZPROPERTY(Category="Math", Desc="Y Position");
    int y;

    __always_inline void print() const {
        std::cout << "X: " << x << " Y: " << y << std::endl;
    }
};

auto vector2_register = register_class<Vector2>("Vector2")
    .property("x", &Vector2::x)
    .property("y", &Vector2::y);

int main() {
    std::unordered_map<std::string, std::any> map = {
        {"x", 2},
        {"y", 3},
    };

    constexpr auto type_name = "Vector2";
    const Type* type = TypeRegistry::instance().get_type_s(type_name);
    std::any instance = type->create();

    for (const auto& [key, value] : map) {
        type->get_property(key)->set_value(instance, value);
    }

    const auto vec2 = std::any_cast<Vector2>(instance);
    vec2.print();
}