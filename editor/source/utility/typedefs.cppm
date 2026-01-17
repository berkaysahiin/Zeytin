module; 

#include <functional>
#include <vector>
#include <string>
#include <unordered_map>
#include <filesystem>
#include <optional>

export module zeytin.utility.typedefs;
export import zeytin.utility.json;

export {
	template<class T>
	using List = std::vector<T>;

	using String = std::string;
	using StringView = std::string_view;

	template<typename Key, typename Value>
	using HashMap = std::unordered_map<Key, Value>;

	template<class Signature>
	using Function = std::function<Signature>;

	using Path = std::filesystem::path;
	using PathView = const std::filesystem::path&;

	template<typename T>
	using Maybe = std::optional<T>;

	template<typename T>
	using MaybeRef = Maybe<std::reference_wrapper<T>>;

	using uint64 = std::uint64_t;
}
