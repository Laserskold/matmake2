#pragma once

#include "filesystem.h"
#include "nativecommands.h"
#include "processedcommand.h"
#include "settings.h"
#include "tasklist.h"
#include <iostream>
#include <map>
#include <mutex>
#include <string>
#include <thread>

class Coordinator {
public:
    enum class RunStatus {
        Normal,
        Failed,
    };

    enum class CoordinatorStatus {
        NotStarted,
        Running,
        Done,
        Failed,
    };

    RunStatus run(std::string command, bool verbose) {
        std::cout << command << std::endl;
        if (std::system(command.c_str())) {
            return RunStatus::Failed;
        }
        else {
            return RunStatus::Normal;
        }
    }

    // Returns true on error
    bool execute(TaskList &tasks, const Settings &settings) {
        using namespace std::chrono_literals;
        _status = CoordinatorStatus::Running;

        {
            auto lock = std::scoped_lock{_todoMutex};
            for (auto &task : tasks) {
                auto state = task->state();
                if (state == TaskState::DirtyReady) {
                    _todo.push_back(task.get());
                    ++_numTasks;
                }
                else if (state == TaskState::DirtyWaiting) {
                    ++_numTasks;
                }
            }
        }

        createDirectories(tasks);

        if (_todo.empty()) {
            std::cout << "Nothing to do...\n";
            return false;
        }

        workers.reserve(settings.numThreads);
        for (size_t i = 0; i < settings.numThreads; ++i) {
            workers.emplace_back([this, i, &settings] {
                if (settings.debugPrint) {
                    std::cout
                        << ("starting thread " + std::to_string(i) + "\n");
                }
                while (_status == CoordinatorStatus::Running) {
                    if (auto task = popTask()) {
                        auto out = task->out();
                        if (out.empty()) {
                            if (settings.debugPrint) {
                                std::cout << " do not build task "
                                          << task->name()
                                          << " because no output files is "
                                             "specified\n";
                            }
                            pushFinished(task, settings.verbose);
                            continue;
                        }

                        auto rawCommand = task->command();

                        if (auto f = native::findCommand(rawCommand)) {
                            if (f(*task) == native::CommandStatus::Failed) {
                                _status = CoordinatorStatus::Failed;
                            }
                            else {
                                pushFinished(task, settings.verbose);
                            }
                        }
                        else if (rawCommand.empty() ||
                                 rawCommand.front() == '[') {
                            throw std::runtime_error{
                                "could not find " + rawCommand + " on target " +
                                task->name()};
                        }
                        else {
                            auto command =
                                ProcessedCommand{rawCommand}.expand(*task);

                            if (!command.empty()) {
                                if (run(command, settings.verbose) ==
                                    RunStatus::Failed) {
                                    _status = CoordinatorStatus::Failed;
                                }
                                else {
                                    task->setState(TaskState::Done);
                                    pushFinished(task, settings.verbose);
                                }
                            }
                        }
                    }
                    else {
                        std::this_thread::sleep_for(10ms);
                    }
                }
            });
        }

        while (_status == CoordinatorStatus::Running) {
            if (_finished.empty()) {
                std::this_thread::sleep_for(10ms);
            }
            else {
                auto lock = std::scoped_lock{_subscriberMutex};
                auto finishedTask = _finished.front();
                for (auto task : finishedTask->subscribers()) {
                    if (task->state() == TaskState::DirtyWaiting) {
                        task->subscribtionNotice(finishedTask);
                        if (task->state() == TaskState::DirtyReady) {
                            pushTask(task);
                        }
                    }
                }
                _finished.erase(_finished.begin(), _finished.begin() + 1);

                ++_numFinished;
                if (_numFinished >= _numTasks) {
                    _status = CoordinatorStatus::Done;
                }
            }
        }

        for (auto &worker : workers) {
            worker.join();
        }

        return _status != CoordinatorStatus::Done;
    }

    [[nodiscard]] Task *popTask() {
        auto lock = std::scoped_lock{_todoMutex};

        if (_todo.empty()) {
            return nullptr;
        }
        else {
            auto task = _todo.front();
            _todo.erase(_todo.begin(), _todo.begin() + 1);
            return task;
        }
    }

    void pushTask(Task *task) {
        auto lock = std::scoped_lock{_todoMutex};
        _todo.push_back(task);
    }

    void pushFinished(Task *task, bool verbose) {
        auto lock = std::scoped_lock{_subscriberMutex};

        if (verbose) {
            std::cout << "task " + task->name() + " finished" << std::endl;
        }
        _finished.push_back(task);
    }

private:
    std::vector<std::thread> workers;
    CoordinatorStatus _status = CoordinatorStatus::NotStarted;

    // Give the threads something to do
    std::mutex _todoMutex;
    std::vector<Task *> _todo;

    // Handle when tasks are finished
    size_t _numTasks;
    size_t _numFinished;
    std::mutex _subscriberMutex;
    std::vector<Task *> _finished;
};
