#pragma once

#include <string>

class Task;

namespace native {

enum class CommandStatus {
    Normal,
    Failed,
};

using CommandType = CommandStatus (*)(const Task &task);

CommandStatus copy(const Task &task);

CommandType findCommand(std::string name);

} // namespace native
