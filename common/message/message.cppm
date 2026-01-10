module;

#include <string>
#include "rapidjson/document.h"

export module zeytin.common.message;

export struct IMessage {
    virtual ~IMessage() = default;
    virtual std::string get_type() const = 0;
    virtual std::string as_json() const = 0;
    virtual bool from_json(const rapidjson::Value& message) = 0;
};
