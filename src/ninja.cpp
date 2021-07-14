#include "ninja.h"
#include "test.h"
#include <fstream>
#include <iostream>

namespace {

void writeNinjaToFile(filesystem::path dir, const TaskList &tasks) {
    std::ofstream file{dir};

    if (!file) {
        throw std::runtime_error{"could not open Makefile for autput " +
                                 dir.string()};
    }

    std::cout << "printing build.ninja..." << std::endl;

    file << "# Ninja file generated with matmake2\n\n";

    file << "builddir = " << dir.parent_path().string() << "\n\n";

    file << "rule run\n";
    file << "    command = $cmd\n\n";

    file << "rule copy\n";
    file << "    command = cp -u $in $out\n\n";

    for (auto &task : tasks) {
        auto rawCommand = task->command();
        auto name = task->name();
        auto out = task->out();

        std::string in;

        if (out.empty()) {
            out = name;
        }

        if (in.empty() && !task->in().empty()) {
            for (auto &i : task->in()) {
                auto o = i->out();
                if (o.empty()) {
                    in += (" " + i->name());
                }
                else {
                    in += (" " + o.string());
                }
            }
        }

        if (in.empty() && rawCommand.empty()) {
            continue;
        }
        if (rawCommand == "none") {
            continue;
        }
        if (rawCommand == "copy") {
            file << "build " << task->out().string() << ": copy " << in
                 << "\n\n";
        }
        else {
            auto command = ProcessedCommand{rawCommand}.expand(*task);

            if (command.empty()) {
                file << "build " << task->name() << ": phony " << in << "\n\n";
            }
            else {
                file << "build " << task->out().string() << ": run " << in
                     << "\n";
                file << "    cmd = " << command << "\n\n";
            }
        }
    }
}

} // namespace

int printNinja(const Settings &settings, const TaskList &tasks) {
    if (settings.target.empty()) {
        throw std::runtime_error{
            "no target specified. Use \"--target\" to specify"};
    }

    if (tasks.empty()) {
        throw std::runtime_error{"could not find target " + settings.target};
    }

    auto root = tasks.find("@" + settings.target);

    if (!root) {
        throw std::runtime_error{"could not find target " + settings.target};
    }

    createDirectories(tasks);

    auto dir = root->dir(BuildLocation::Intermediate) / "build.ninja";

    writeNinjaToFile(dir, tasks);

    std::cout << "running ninja..." << std::endl;

    if (!settings.skipBuild) {
        std::cout.flush();
        auto status = system(("ninja -f " + dir.string()).c_str());

        if (status) {
            std::cout << "failed...\n";
            return status;
        }
        else {
            std::cout << "done...\n\n";
        }

        if (settings.command != Command::BuildAndTest || status) {
            return status;
        }
    }

    if (settings.command == Command::BuildAndTest) {
        return test(tasks, settings);
    }

    return 0;
}
