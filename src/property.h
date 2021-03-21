#pragma once

#include "task.h"
#include "json/json.h"
#include <algorithm>
#include <string>

struct Property {
    Property() = default;
    Property(const Property &) = default;
    Property(Property &&) = default;
    Property &operator=(const Property &) = default;
    Property &operator=(Property &&) = default;
    Property(std::string value)
        : values({std::move(value)}) {}
    Property(std::vector<std::string> values)
        : values(std::move(values)) {}

    Property(const Json &json) {
        if (json.type == Json::Array) {
            values.reserve(json.size());
            for (auto &child : json) {
                values.push_back(child.string());
            }
        }
        else {
            values.push_back(json.string());
        }
    }

    std::vector<std::string> values;
    Json::Position pos;

    std::string value() const {
        if (values.size() == 0) {
            throw std::runtime_error{"could not find value for property"};
        }
        else if (values.size() > 1) {
            throw std::runtime_error{
                "Expected single property: " + std::string{pos} + " -> " +
                values.front()};
        }
        return values.front();
    }

    //! Convert to a filesystem path with the right separater type
    filesystem::path path() const {
        auto value = this->value();
        return normalizePath(value);
    }

    std::string concat() const {
        if (values.empty()) {
            return {};
        }

        std::ostringstream ss;
        for (auto &value : values) {
            ss << value << " ";
        }

        auto str = ss.str();

        str.pop_back();
        return str;
    }

    friend std::ostream &operator<<(std::ostream &stream, const Property prop) {
        if (prop.values.empty())
            ;
        else if (prop.values.size() > 1) {
            stream << "[ ";
            for (auto &value : prop.values) {
                stream << value << ", ";
            }
            stream << " ]";
        }
        else {
            stream << prop.values.front();
        }

        return stream;
    }
};
