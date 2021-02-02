#include "task.h"
#include "json/json.h"

void Task::parse(const Json &jtask) {
    auto jsonFind = [&jtask](std::string name) -> const Json * {
        auto f = jtask.find(name);
        if (f != jtask.end()) {
            return &*f;
        }
        else {
            return nullptr;
        }
    };
    if (auto f = jsonFind("name")) {
        name(f->string());
    }
    if (auto f = jsonFind("out")) {
        out(f->string());
    }
    if (auto f = jsonFind("command")) {
        command(f->string());
    }
    if (auto f = jsonFind("commands")) {
        if (f->type == Json::Object) {
            std::map<std::string, std::string> commands;
            for (auto &child : *f) {
                commands[child.name] = child.value;
            }

            this->commands(std::move(commands));
        }
    }
    if (auto f = jsonFind("dir")) {
        dir(f->string());
    }
    if (auto f = jsonFind("depfile")) {
        depfile(f->string());
    }
}

Json Task::dump() {
    return Json{};
}

void Task::print(size_t indentation) {
    auto indent = [indentation]() -> std::ostream & {
        for (size_t i = 0; i < indentation; ++i) {
            std::cout << "  ";
        }
        return std::cout;
    };

    indent();
    std::cout << name() << " " << out() << "\n";

    try {
        {
            auto command = this->command();
            indent();
            std::cout << "command: "
                      << ProcessedCommand{command}.expandCommand(*this) << "\n";

            indent();
            std::cout << "raw: " << command << "\n";
        }
    }
    catch (...) {
    }

    indent() << "dir: " << dir() << "\n";

    {
        auto commands = this->commands();
        if (!commands.empty()) {
            indent() << "defined commands\n";
        }

        for (const auto &command : commands) {
            indent() << "  \"" << command.first << " = " << command.second
                     << "\"\n";
        }
    }

    if (!in().empty()) {
        indent() << "in:\n";
    }
    for (auto &in : in()) {
        in->print(indentation + 1);
    }
}
