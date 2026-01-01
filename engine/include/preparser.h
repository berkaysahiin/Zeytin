#pragma once

#define PROPERTY(...) [[clang::annotate(#__VA_ARGS__)]]
