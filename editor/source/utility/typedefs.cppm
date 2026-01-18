module; 

#include <functional>
#include <vector>
#include <string>
#include <unordered_map>
#include <filesystem>
#include <optional>
#include <type_traits>

export module zeytin.utility.typedefs;
export import zeytin.utility.json;
import zeytin.logger;

export {
	using uint64 = std::uint64_t;

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

	// == ref ==

	template<typename T>
	using ConstRef = const T&;

	template<typename T>
	using Ref = T&;

	//  == maybe ==
	template<typename T>
	using Maybe = std::optional<T>;

	template<typename T>
	using MaybeRef = Maybe<std::reference_wrapper<T>>;

	template<typename T>
	using MaybeConstRef = Maybe<std::reference_wrapper<const T>>;
}
