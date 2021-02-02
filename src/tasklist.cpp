#include "tasklist.h"
#include "parsedepfile.h"
#include "json/json.h"

namespace {

void connectTasks(TaskList &list, const Json &json) {
    for (size_t i = 0; i < json.size(); ++i) {
        auto &task = list.at(i);
        auto &data = json.at(i);

        if (auto f = data.find("in"); f != data.end()) {
            if (f->type == Json::Array) {
                for (auto &value : *f) {
                    auto ftask = list.find(value.string());
                    if (ftask) {
                        task.pushIn(ftask);
                    }
                    else {
                        throw std::runtime_error{"could not find task " +
                                                 value.string()};
                    }
                }
            }
            else {
                task.pushIn(list.find(f->value));
            }
        }

        if (auto f = data.find("deps"); f != data.end()) {
            if (f->type == Json::Array) {
                for (auto &value : *f) {
                    auto ftask = list.find(value.string());
                    if (ftask) {
                        task.pushTrigger(ftask);
                    }
                    else {
                        throw std::runtime_error{"could not find task " +
                                                 value.string()};
                    }
                }
            }
            else {
                task.pushIn(list.find(f->value));
            }
        }
    }
}

void calculateState(TaskList &list) {
    for (auto &task : list) {
        auto depfile = task.depfile();
        if (depfile.empty()) {
            continue;
        }
        for (auto &f : parseDepFile(depfile).deps) {
            task.pushTrigger(list.find(f.string()));
        }
    }

    for (auto &task : list) {
        task.updateState();
    }
}

} // namespace

std::unique_ptr<TaskList> parseTasks(filesystem::path path) {
    auto list = std::make_unique<TaskList>();
    auto json = Json{};
    auto file = std::ifstream{path};

    if (!file.is_open()) {
        throw std::runtime_error{"could not load tasks from " + path.string()};
    }
    file >> json;

    if (json.type != Json::Array) {
        throw std::runtime_error{"expected array in " + path.string()};
    }

    list->reserve(json.size());

    for (const auto &jtask : json) {
        list->emplace().parse(jtask);
    }

    connectTasks(*list, json);

    calculateState(*list);

    return list;
}

void printFlat(const TaskList &list) {
    for (auto &task : list) {
        std::cout << "task: name = " << task.name() << "\n";
        std::cout << "  out = " << task.out() << "\n";
        if (task.parent()) {
            std::cout << "  parent = " << task.parent()->out() << "\n";
        }

        {
            auto &in = task.in();
            for (auto &i : in) {
                std::cout << "  in: " << i->out() << "\n";
            }
        }
    }
}
