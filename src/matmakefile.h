#pragma once

#include "property.h"
#include "json/json.h"
#include <iostream>

// Contains data loaded from json object
class MatmakeNode {
    std::map<std::string, Property> _properties;
    std::string _name;
    std::map<std::string, std::string> _commands;

public:
    MatmakeNode(const MatmakeNode &) = default;
    MatmakeNode(MatmakeNode &&) = default;
    MatmakeNode &operator=(const MatmakeNode &) = default;
    MatmakeNode &operator=(MatmakeNode &&) = default;

    MatmakeNode(const Json &json, std::string_view targetName);

    // Merge nodes (used when two nodes has the same name)
    bool merge(const MatmakeNode &other, std::string_view targetName);

    const Property *property(std::string name) const {
        if (auto f = _properties.find(name); f != _properties.end()) {
            return &f->second;
        }
        else {
            return nullptr;
        }
    }

    const auto &ocommands() const {
        return _commands;
    }

    std::string_view name() const {
        return _name;
    }

    void print(std::ostream &stream) {
        stream << "target: " << _name << "\n";
        for (auto &property : _properties) {
            stream << "  " << property.first << " = " << property.second
                   << "\n";
        }
    }

    bool isRoot() const {
        constexpr auto rootStr = std::string_view{"root"};
        if (auto f = property("root")) {
            return f->values.size() == 1 && f->values.front() == rootStr;
        }
        return false;
    }
};

class MatmakeFile {
public:
    // @param json
    MatmakeFile(const Json &json, std::string_view targetName = "");

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
