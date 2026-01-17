module;

#include <filesystem>

export module zeytin.utility.path;

export {
	using Path = std::filesystem::path;
	using PathView = const std::filesystem::path&;
}
