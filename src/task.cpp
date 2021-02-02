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
    if (auto f = jsonFind("c++")) {
        cxx(f->string());
    }
}

Json Task::dump() {
    auto json = Json{};

    auto attachValue = [&json](const std::string name, std::string value) {
        if (!value.empty()) {
            json[name] = value;
        }
    };

    attachValue("name", _name);
    attachValue("out", _out.string());
    attachValue("dir", _dir.string());
    attachValue("depfile", _depfile.string());
    attachValue("command", _command);
    attachValue("c++", _cxx.string());

    if (!_in.empty()) {
        auto j = Json{Json::Array};

        for (auto i : _in) {
            auto ij = Json{Json::String};
            if (i->_name.empty()) {
                ij.string(i->_out.string());
            }
            else {
                ij.string("@" + i->_name);
            }
            j.push_back(ij);
        }
    }

    return json;
}

void Task::print(bool verbose, size_t indentation) {
    auto indent = [indentation]() -> std::ostream & {
        for (size_t i = 0; i < indentation; ++i) {
            std::cout << "  ";
        }
        return std::cout;
    };

    indent();
    std::cout << name() << " " << out() << "\n";

    if (_state != TaskState::Raw) {
        try {
            {
                auto command = this->command();
                indent();
                std::cout << "command: "
                          << ProcessedCommand{command}.expand(*this) << "\n";

                indent();
                std::cout << "raw: " << command << "\n";
            }
        }
        catch (...) {
        }
    }

    indent() << "dir: " << dir() << "\n";

    indent() << "exists: " << (filesystem::exists(out()) ? "yes" : "no")
             << "\n";

    indent() << "dirty: " << (isDirty() ? "yes" : "no") << "\n";

    if (verbose) {
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
        for (auto &in : in()) {
            in->print(verbose, indentation + 1);
        }
    }

    if (!triggers().empty()) {
        indent() << "in:\n";
        for (auto &trigger : triggers()) {
            trigger->print(verbose, indentation + 1);
        }
    }

    indent() << " -- \n";
}
