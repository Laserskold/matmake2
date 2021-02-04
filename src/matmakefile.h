#pragma once

#include "json/json.h"

// Contains data loaded from json object
class MatmakeNode {
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
                throw std::runtime_error{"Expected single property: " +
                                         std::string{pos}};
            }
            return values.front();
        }

        friend std::ostream &operator<<(std::ostream &stream,
                                        const Property prop) {
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

    std::map<std::string, Property> _properties;
    std::string _name;

public:
    MatmakeNode(const Json &json) {
        if (json.type != Json::Object) {
            throw std::runtime_error{"Json: Wrong type when expected object " +
                                     std::string{json.pos}};
        }

        for (auto &child : json) {
            auto &prop = _properties[child.name] = Property{child};
            if (child.name == "name") {
                _name = child.value;
            }
        }
    }

    const Property *property(std::string name) const {
        if (auto f = _properties.find(name); f != _properties.end()) {
            return &f->second;
        }
        else {
            return nullptr;
        }
    }

    const std::string &name() const {
        return _name;
    }

    void print(std::ostream &stream) {
        stream << "target: " << _name << "\n";
        for (auto &property : _properties) {
            stream << "  " << property.first << " = " << property.second
                   << "\n";
        }
    }
};

class MatmakeFile {
public:
    MatmakeFile(const Json &json) {
        if (json.type != Json::Array) {
            throw std::runtime_error{"Json: Wrong type when expected array " +
                                     std::string{json.pos}};
        }

        _nodes.reserve(json.size());

        for (auto &j : json) {
            _nodes.emplace_back(j);
        }
    }

    void print(std::ostream &stream = std::cout) {
        for (auto &child : _nodes) {
            child.print(stream);
        }
    }

    const std::vector<MatmakeNode> nodes() const {
        return _nodes;
    }

    const MatmakeNode *find(std::string name) const {
        for (auto &child : _nodes) {
            if (child.name() == name) {
                return &child;
            }
        }

        return nullptr;
    }

    std::vector<MatmakeNode> _nodes;
};
