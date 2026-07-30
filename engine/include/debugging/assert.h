#pragma once

#include <cassert>

#define ASSERT(EXPR, ...) \
	do { \
		const bool expr = static_cast<bool>((EXPR)); \
		if (!expr) { \
			log_error(__VA_ARGS__); \
		} \
		assert(expr); \
	} while (false)
