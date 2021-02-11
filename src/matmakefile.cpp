#include "matmakefile.h"
#include "defaultfile.h"

MatmakeFile::MatmakeFile(const Json &json) {
    if (json.type != Json::Array) {
        throw std::runtime_error{"Json: Wrong type when expected array " +
                                 std::string{json.pos}};
    }

    _nodes.reserve(json.size());

    for (auto &j : json) {
        _nodes.emplace_back(j);
    }

    //! Put default targets on end if they do not exist
    auto def = defaultCompiler();

    std::vector<MatmakeNode> defaultNodes;
    defaultNodes.reserve(def.size());

    for (auto &j : def) {
        defaultNodes.emplace_back(j);
    }

    for (auto &n : defaultNodes) {
        if (!find(n.name())) {
            _nodes.emplace_back(std::move(n));
        }
    }
}
