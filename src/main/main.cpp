// copyright Mattias Larsson Sköld 2021

#include "coordinator.h"
#include "createtasks.h"
#include "filesystem.h"
#include "makefile.h"
#include "matmakefile.h"
#include "msvcenvironment.h"
#include "parsematmakefile.h"
#include "settings.h"
#include "tasklist.h"
#include "json/json.h"

namespace {

TaskList createTasksFromMatmakefile(const Settings &settings) {
    auto getJson = [&]() -> Json {
        if (filesystem::exists("Matmakefile")) {
            return parseMatmakefile("Matmakefile");
        }
        else if (filesystem::exists("matmake.json")) {
            return Json::LoadFile("matmake.json");
        }
        else {
            std::cerr << "no matmakefile found in directory\n";
            return {};
        }
    };

    auto matmakeFile = MatmakeFile{getJson()};

    if (settings.debugPrint) {
        matmakeFile.print(std::cout);
    }

    return createTasks(matmakeFile, settings.target);
}

int parseTasksCommand(const Settings settings) {
    if (!settings.taskFile.empty()) {
        filesystem::current_path(settings.taskFile.parent_path());

        auto tasks = parseTasks(settings.taskFile.filename());

        if (settings.printTasks) {
            printFlat(*tasks);
        }

        if (settings.printTree) {
            std::cout << "\n"
                         "treeview\n"
                         "==================\n";
            auto &root = *tasks->find("@g++");
            root.print(settings.verbose);
        }

        std::cout << "building... \n";

        if (!settings.skipBuild) {
            auto coordinator = Coordinator{};

            auto status = coordinator.execute(*tasks, settings);

            if (status) {
                std::cout << "failed...\n";
            }
            else {
                std::cout << "done...\n";
            }

            return status;
        }
    }

    std::cerr << "could not open task file\n";

    return 1;
}

int execute(std::string filename, filesystem::path path) {
    auto originalPath = filesystem::absolute(filesystem::current_path());
    filesystem::current_path(path);

    if constexpr (getOs() == Os::Linux) {
        filename = "./" + filename;
    }

    auto res = std::system(filename.c_str());

    filesystem::current_path(originalPath);

    return res;
}

int test(const TaskList &tasks, const Settings &settings) {
    std::vector<const Task *> tests;

    size_t failedTests = 0;
    size_t numTests = 0;

    std::vector<std::string> results;

    for (auto &task : tasks) {
        if (task->isTest()) {
            ++numTests;
            auto exe = task->out();
            auto path = exe.parent_path();
            auto logFile =
                task->dir(BuildLocation::Intermediate) / task->name();
            logFile.replace_extension(".txt");
            logFile = filesystem::absolute(logFile);
            auto name = task->name();

            auto command = exe.filename().string() + " > " + logFile.string();
            std::cout << name << ":";
            std::cout.width(name.size() < 40 ? 40 - name.size() : 0);
            std::cout.fill(' ');
            std::cout << " running in " << path << std::endl;
            if (execute(command, path)) {
                ++failedTests;

                std::cout << "\n" << name << " failed:\n";
                std::cout << "---------------------------------------------\n";

                auto file = std::ifstream{logFile};
                for (std::string line; getline(file, line);) {
                    std::cout << line << "\n";
                }
                std::cout << "--------- end test --------------------------\n";
                std::cout.flush();

                results.push_back("failed            " + exe.string());
            }
            else {
                results.push_back("success           " + exe.string());
            }
        }
    }

    std::cout << "\n\n==== Test summary: ========================== \n";

    for (auto &result : results) {
        std::cout << result << "\n";
    }

    std::cout << "\n";

    std::cout << failedTests << " failed of " << numTests << " tests "
              << std::endl;

    return failedTests > 0;
}

void outputCompileCommands(const TaskList &tasks) {
    auto json = Json{Json::Array};

    auto dir = filesystem::absolute(filesystem::current_path());
    for (auto &task : tasks) {
        if (getType(task->out()) != SourceType::Object) {
            continue;
        }
        auto command = task->command();
        auto taskJson = Json{Json::Object};

        taskJson["directory"] = dir.string();
        taskJson["command"] = ProcessedCommand{command}.expand(*task);
        auto source = task->findSource();
        if (!source) {
            std::cerr << "could not find source file for " << task->out();
            continue;
        }
        auto file = source->out().string();
        if (file.rfind("./", 0) == 0) {
            file = file.substr(2);
        }
        taskJson["file"] = file;

        json.push_back(std::move(taskJson));
    }

    std::ofstream{"compile_commands.json"} << json;
}

int build(const Settings &settings) {
    auto tasks = createTasksFromMatmakefile(settings);

    if (settings.target.empty()) {
        throw std::runtime_error{
            "no target specified. Use \"--target\" to specify"};
    }

    if (tasks.empty()) {
        throw std::runtime_error{"could not find target " + settings.target};
    }

    if (settings.printTree) {
        std::cout << "\n"
                     "treeview\n"
                     "==================\n";
        auto root = tasks.find("@" + settings.target);

        if (!root) {
            throw std::runtime_error{"could not find target " +
                                     settings.target};
        }

        root->print(settings.verbose);
        std::cout.flush();
    }

    if (settings.outputCompileCommands) {
        outputCompileCommands(tasks);
    }

    if (!settings.skipBuild) {
        auto coordinator = Coordinator{};
        auto status = coordinator.execute(tasks, settings);

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

int list(const Settings &settings) {
    auto json = [] {
        if (filesystem::exists("Matmakefile")) {
            return parseMatmakefile("Matmakefile");
        }

        return Json::LoadFile("matmake.json");
    }();

    auto matmakeFile = MatmakeFile{json};

    for (auto &node : matmakeFile.nodes()) {
        if (auto p = node.property("command"); p && p->concat() == "[root]") {
            std::cout << node.name() << "\n";
        }
    }

    return 0;
}

int clean(const Settings settings) {
    auto tasks = createTasksFromMatmakefile(settings);

    for (auto &task : tasks) {
        if (task->clean() && settings.verbose) {
            std::cout << "removed " << task->out() << "\n";
        }
    }

    return 0;
}

} // namespace

int main(int argc, char **argv) {
    const auto settings = Settings{argc, argv};

    try {
        if (settings.useMsvcEnvironment) {
            setMsvcEnvironment();
        }

        switch (settings.command) {
        case Command::ParseTasks:
            return parseTasksCommand(settings);
            break;
        case Command::Build:
        case Command::BuildAndTest: {
            //            return build(settings);
            return printMakefile(settings,
                                 createTasksFromMatmakefile(settings));
        } break;
        case Command::List: {
            return list(settings);
        } break;
        case Command::Clean: {
            return clean(settings);
        } break;
        }
    }
    catch (std::runtime_error &e) {
        std::cerr << "error: " << e.what() << "\n";
        return 1;
    }

    return 0;
}
