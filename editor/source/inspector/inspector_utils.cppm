module;

#include "imgui.h"
#include "rapidjson/document.h"

#include <optional>
#include <string>
#include <limits>

export module zeytin.inspector.utils;

export struct IntBounds {
    int min = std::numeric_limits<int>::min();
    int max = std::numeric_limits<int>::max();
    bool use = false;
};

export struct FloatBounds {
    float min = std::numeric_limits<float>::lowest();
    float max = std::numeric_limits<float>::max();
    bool use = false;
};

export std::optional<int> parse_optional_int(const std::optional<std::string>& value) {
    if (!value.has_value() || value->empty()) {
        return std::nullopt;
    }
    try {
        size_t processed = 0;
        int parsed = std::stoi(*value, &processed);
        if (processed != value->size()) {
            return std::nullopt;
        }
        return parsed;
    } catch (...) {
        return std::nullopt;
    }
}

export std::optional<float> parse_optional_float(const std::optional<std::string>& value) {
    if (!value.has_value() || value->empty()) {
        return std::nullopt;
    }
    try {
        size_t processed = 0;
        float parsed = std::stof(*value, &processed);
        if (processed != value->size()) {
            return std::nullopt;
        }
        return parsed;
    } catch (...) {
        return std::nullopt;
    }
}

export IntBounds make_int_bounds(const std::optional<std::string>& min_annotation,
                                 const std::optional<std::string>& max_annotation) {
    IntBounds bounds;
    const auto min_value = parse_optional_int(min_annotation);
    const auto max_value = parse_optional_int(max_annotation);
    bounds.use = min_value.has_value() || max_value.has_value();
    if (min_value.has_value()) {
        bounds.min = min_value.value();
    }
    if (max_value.has_value()) {
        bounds.max = max_value.value();
    }
    if (bounds.min > bounds.max) {
        std::swap(bounds.min, bounds.max);
    }
    return bounds;
}

export FloatBounds make_float_bounds(const std::optional<std::string>& min_annotation,
                                     const std::optional<std::string>& max_annotation) {
    FloatBounds bounds;
    const auto min_value = parse_optional_float(min_annotation);
    const auto max_value = parse_optional_float(max_annotation);
    bounds.use = min_value.has_value() || max_value.has_value();
    if (min_value.has_value()) {
        bounds.min = min_value.value();
    }
    if (max_value.has_value()) {
        bounds.max = max_value.value();
    }
    if (bounds.min > bounds.max) {
        std::swap(bounds.min, bounds.max);
    }
    return bounds;
}

export void show_tooltip_if_hovered(const std::optional<std::string>& tooltip) {
    if (!tooltip.has_value() || tooltip->empty()) {
        return;
    }
    if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip("%s", tooltip->c_str());
    }
}

export std::optional<std::string> get_key_type_from_value(const rapidjson::Value& value) {
    if (value.IsInt()) {
        return "int";
    }
    if (value.IsFloat() || value.IsDouble()) {
        return "float";
    }
    if (value.IsBool()) {
        return "bool";
    }
    if (value.IsString()) {
        return "string";
    }

    return std::nullopt;
}
