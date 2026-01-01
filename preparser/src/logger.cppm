module;

#include <format>
#include <iostream>

export module preparser.logger;

export template<typename... Args>
void log(std::format_string<Args...> fmt, Args&&... args) {
	std::cout << std::format(fmt, std::forward<Args>(args)...) << std::endl;;
}
