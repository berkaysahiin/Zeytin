module;

#include <optional>

export module zeytin.utility.maybe;

export {
	template<typename T>
	using Maybe = std::optional<T>;

	template<typename T>
	using MaybeRef = Maybe<std::reference_wrapper<T>>;
}

