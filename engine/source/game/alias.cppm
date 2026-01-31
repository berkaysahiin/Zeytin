module;

#include <type_traits>

export module zeytin.game.alias;

export {
	template<typename... Ts>
	struct Alias {};

	template<typename T, typename... Ts>
	constexpr bool alias_contains_v = (std::is_same_v<T, Ts> || ...);
}
