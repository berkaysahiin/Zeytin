#pragma once

#define PROPERTY(...) [[clang::annotate(#__VA_ARGS__)]]

// used for taging virtual component
struct VirtualComponentTag {};

#define VIRTUAL_COMPONENT  private: [[no_unique_address]] VirtualComponentTag ___virtual_component_tag; public:
