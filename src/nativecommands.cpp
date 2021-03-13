#include "nativecommands.h"
#include "filesystem.h"
#include "task.h"
#include <iostream>

native::CommandType native::findCommand(std::string name) {
    if (name.empty()) {
        return {};
    }

    if (name.front() == '[' && name.back() == ']') {
        name = name.substr(1, name.size() - 2);
    }

    if (name == "copy") {
        return &copy;
    }
    if (name == "none") {
        return [](const Task &) { return native::CommandStatus::Normal; };
    }
    else {
        return nullptr;
    }
}

native::CommandStatus native::copy(const Task &task) {
    std::error_code ec;

    auto in = task.in().front()->out();
    auto out = task.out();

    std::cout << (in.string() + " --> " + out.string()) << std::endl;

    if (filesystem::equivalent(in, out)) {
        return CommandStatus::Normal;
    }

    filesystem::copy(in, out, filesystem::copy_options::overwrite_existing, ec);

    if (ec) {
        return CommandStatus::Failed;
    }
    else {
        return CommandStatus::Normal;
    }
}
