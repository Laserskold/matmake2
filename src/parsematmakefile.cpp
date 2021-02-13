#include "parsematmakefile.h"
#include "line.h"

namespace {} // namespace

Json parseMatmakefile(std::istream &file) {

    auto json = Json{Json::Array};

    auto lastTarget = Json{};
    auto lastIndent = size_t{0};

    auto finishTarget = [&lastTarget, &json] {
        if (!lastTarget.empty()) {
            json.push_back(lastTarget);
        }
        lastTarget = Json{Json::Object};
    };

    for (auto line = Line{file}; line.type != Line::End; line = Line{file}) {
        switch (line.type) {
        case Line::Normal:
            if (lastTarget.size() > 0 && line.indent > 0) {
                lastTarget.type = Json::Array;
                lastTarget.back().emplace_back(line.name);
            }
            else {
                finishTarget();
                lastTarget.name = line.name;
            }
            break;
        case Line::Assignment:
            if (line.value.size() == 1) {
                lastTarget[line.name] = line.value.front();
            }
            else if (line.value.size() > 1) {
                lastTarget[line.name].assign(line.value.begin(),
                                             line.value.end());
            }
            break;
        case Line::UnfinishedAssignment:
        default:
            lastTarget[line.name];
            break;
        }
        lastIndent = line.indent;
    }

    finishTarget();

    return json;
}

Json parseMatmakefile(filesystem::path path) {
    auto file = std::ifstream{path};

    if (!file.is_open()) {
        throw std::runtime_error{"could not open matmakefile: " +
                                 path.string()};
    }

    return parseMatmakefile(file);
}
