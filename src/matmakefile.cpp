#include "matmakefile.h"
#include "defaultfile.h"
#include <algorithm>

namespace {

struct PropertyPattern {
    PropertyPattern(std::string_view str) {
        auto f = str.find(':');
        if (f == std::string_view::npos) {
            name = str;
            return;
        }

        name = str.substr(f + 1);
        target = str.substr(0, f);

        if (!target.empty() && target.front() == '!') {
            inverted = true;
            target = target.substr(1);
        }
    }

    bool isPattern() {
        return !target.empty();
    }

    std::string_view name;
    std::string_view target;
    bool inverted = false;
};

} // namespace

MatmakeFile::MatmakeFile(const Json &json, std::string_view targetName) {
    if (json.type != Json::Array) {
        throw std::runtime_error{"Json: Wrong type when expected array " +
                                 std::string{json.pos}};
    }

    //! Put default targets on end if they do not exist
    auto def = defaultCompiler();

    std::vector<MatmakeNode> defaultNodes;
    defaultNodes.reserve(def.size());

    for (auto &j : def) {
        defaultNodes.emplace_back(j, targetName);
    }

    for (auto &n : defaultNodes) {
        _nodes.emplace_back(std::move(n));
    }

    _nodes.reserve(json.size() + _nodes.size());

    auto nonConstFind = [this](std::string_view name) -> MatmakeNode * {
        for (auto &node : _nodes) {
            if (node.name() == name) {
                return &node;
            }
        }

        return nullptr;
    };

    // Try to apply new node to other nodes
    auto tryMerge = [this, targetName](const MatmakeNode &newNode) {
        bool success = false;
        for (auto &node : _nodes) {
            if (node.merge(newNode, targetName)) {
                success = true;
            }
        }
        return success;
    };

    for (auto &j : json) {
        auto node = MatmakeNode{j, targetName};
        if (!tryMerge(node)) {
            _nodes.emplace_back(std::move(node));
        }
    }
}

MatmakeNode::MatmakeNode(const Json &json, std::string_view targetName) {
    if (json.type != Json::Object) {
        throw std::runtime_error{"Json: Wrong type when expected object " +
                                 std::string{json.pos}};
    }

    for (auto &child : json) {
        if (child.name == "name") {
            _name = child.value;
        }

        if (child.name == "commands") {
            for (auto &command : child) {
                _commands[command.name] = command.string();
            }
        }
        else {
            auto pattern = PropertyPattern{child.name};

            // != as xor
            if (pattern.inverted !=
                (pattern.target.empty() || (pattern.target == targetName))) {
                _properties[std::string{pattern.name}].append(
                    Property{child}.values);
            }
        }
    }
}

bool MatmakeNode::merge(const MatmakeNode &other, std::string_view targetName) {
    if (other.name().substr(0, 1) == "!") {
        // A name with a exclamation mark before will match all targets
        // that does not contain that name, and is a root node
        if (other.name().substr(1) == name()) {
            return false;
        }
        if (!isRoot()) {
            return false;
        }
    }
    else if (other.name() != name()) {
        return false;
    }

    constexpr auto nameStr = std::string_view{"name"};

    for (auto &othersProperty : other._properties) {
        auto pattern = PropertyPattern{othersProperty.first};

        // Ignore this
        if (othersProperty.first == nameStr) {
            continue;
        }

        if (pattern.inverted != (pattern.target == targetName)) {
            auto &mergedProperty = _properties[othersProperty.first];
            mergedProperty.append(othersProperty.second.values);
        }
    }

    return true;
}
