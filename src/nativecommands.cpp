#include "nativecommands.h"
#include "filesystem.h"
#include "task.h"

native::CommandType native::findCommand(std::string name) {
    if (name == "copy") {
        return &copy;
    }
    else {
        return nullptr;
    }
}

native::CommandStatus native::copy(const Task &task) {
    std::error_code ec;
    filesystem::copy(task.in().front()->out(),
                     task.out(),
                     filesystem::copy_options::overwrite_existing,
                     ec);

    if (ec) {
        return CommandStatus::Failed;
    }
    else {
        return CommandStatus::Normal;
    }
}
