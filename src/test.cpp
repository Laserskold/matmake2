#include "test.h"
#include "execute.h"
#include <fstream>
#include <iostream>
#include <vector>

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
