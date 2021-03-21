#include "nativecommands.h"
#include "filesystem.h"
#include "task.h"
#include <algorithm>
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

#ifdef MATMAKE_USING_WINDOWS

    {
        auto str = in.string();
        std::replace(str.begin(), str.end(), '/', '\\');
        in = str;
    }

    {
        auto str = out.string();
        std::replace(str.begin(), str.end(), '/', '\\');
        out = str;
    }

#endif

    std::cout << (in.string() + " --> " + out.string()) << std::endl;

#ifndef MATMAKE_USING_WINDOWS
    // This causes a crash on msvc..
    if (filesystem::equivalent(in, out)) {
        return CommandStatus::Normal;
    }
#endif

    filesystem::copy(in, out, filesystem::copy_options::overwrite_existing, ec);

    if (ec) {
        std::cerr << "error: Failed to copy " << in << " --> " << out
                  << std::endl;
        return CommandStatus::Failed;
    }
    else {
        return CommandStatus::Normal;
    }
}
