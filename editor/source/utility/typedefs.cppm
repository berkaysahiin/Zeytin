module; 

#include <functional>
#include <vector>
#include <string>
#include <unordered_map>

export module zeytin.utility.typedefs;
export import zeytin.utility.path;
export import zeytin.utility.maybe;
export import zeytin.utility.json;
export import zeytin.utility.string;

export {
	template<class T>
	using List = std::vector<T>;

	using String = std::string;

	template<typename Key, typename Value>
	using HashMap = std::unordered_map<Key, Value>;

	template<class Signature>
	using Function = std::function<Signature>;
}
