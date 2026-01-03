module;

#include <variant>
#include <string>
#include <vector>

export module zeytin.editor.property;

struct Property {
	using Value = std::variant<int, float, bool, std::string, std::vector<Property>>;
};
