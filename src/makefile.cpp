#include "makefile.h"
#include "createtasks.h"
#include "tasklist.h"
#include <fstream>
#include "test.h"

namespace {

void writeToFile(filesystem::path dir, const TaskList &tasks) {

    std::ofstream file{dir};

    if (!file) {
        throw std::runtime_error{"could not open Makefile for autput " +
                                 dir.string()};
    }

    std::cout << "printing makefile..." << std::endl;

    for (auto &task : tasks) {
        //        auto in = task->property("in");
        auto rawCommand = task->command();
        auto name = task->name();
        auto out = task->out();

        std::string in;

        if (out.empty()) {
            out = name;
        }

        //        while (!in.empty() && in.back() == ' ') {
        //            in.pop_back();
        //        }

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
        file << out.string() << ": " << in << "\n";
        if (rawCommand == "none") {
            continue;
        }
        if (rawCommand == "copy") {
            file << "\t"
                 << "cp -u " << in << " " << task->out().string() << "\n";
            //                file << "\t" << in << " -> " <<
            //                task->out().string() << "\n";
        }
        else {
            auto command = ProcessedCommand{rawCommand}.expand(*task);
            file << "\t" << command << "\n";
        }
    }
}

} // namespace

int printMakefile(const Settings &settings, const TaskList &tasks) {
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

    //    root->print(settings.verbose);

    createDirectories(tasks);

    auto dir = root->dir(BuildLocation::Intermediate) / "Makefile";

    writeToFile(dir, tasks);

    std::cout << "running makefile..." << std::endl;

    if (!settings.skipBuild) {
        std::cout.flush();
        auto status = system(("make -j -f " + dir.string()).c_str());

        if (status) {
            std::cout << "failed...\n";
        }
        else {
            std::cout << "done...\n";
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
