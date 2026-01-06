module;

#include <vector>

export module preparser.rttr_generator;
import preparser.types;

export void generate_rttr_registration(const std::vector<ComponentInfo>& components);
