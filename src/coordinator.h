#pragma once

#include "filesystem.h"
#include "nativecommands.h"
#include "tasklist.h"
#include <map>
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

    RunStatus run(std::string command) {
        std::cout << ("running " + command + "\n");
        std::cout.flush();
        if (std::system(command.c_str())) {
            return RunStatus::Failed;
        }
        else {
            return RunStatus::Normal;
        }
    }

    void createDirectories(const TaskList &tasks) const {
        auto directories = std::map<filesystem::path, int>{};

        for (auto &task : tasks) {
            ++directories[task.out().parent_path()];
        }

        for (auto &it : directories) {
            if (!filesystem::exists(it.first)) {
                filesystem::create_directories(it.first);
            }
            else if (!filesystem::is_directory(it.first)) {
                throw std::runtime_error{"expected " + it.first.string() +
                                         " to be a directory "
                                         "but it is a file"};
            }
        }
    }

    void execute(TaskList &tasks) {
        using namespace std::chrono_literals;
        _status = CoordinatorStatus::Running;

        auto nThreads = std::thread::hardware_concurrency();
        //        auto nThreads = 1;

        {
            auto lock = std::scoped_lock{_todoMutex};
            for (auto &task : tasks) {
                auto state = task.state();
                if (state == TaskState::DirtyReady) {
                    _todo.push_back(&task);
                    ++_numTasks;
                }
                else if (state == TaskState::DirtyWaiting) {
                    ++_numTasks;
                }
            }
        }

        createDirectories(tasks);

        if (_todo.empty()) {
            throw std::runtime_error{
                "cannot build anything. No task is possible to build\n"};
        }

        workers.reserve(nThreads);
        for (size_t i = 0; i < nThreads; ++i) {
            workers.emplace_back([this, i] {
                std::cout << "starting thread " << i << "\n";
                while (_status == CoordinatorStatus::Running) {
                    if (auto task = popTask()) {
                        auto out = task->out();
                        if (out.empty()) {
                            std::cout
                                << " do not build task " << task->name()
                                << " because no output files is specified\n";
                            pushFinished(task);
                            continue;
                        }

                        auto rawCommand = task->command();

                        if (auto f = native::findCommand(rawCommand)) {
                            if (f(*task) == native::CommandStatus::Failed) {
                                _status = CoordinatorStatus::Failed;
                            }
                            else {
                                pushFinished(task);
                            }
                        }
                        else {
                            auto command =
                                ProcessedCommand{rawCommand}.expand(*task);

                            if (!command.empty()) {
                                if (run(command) == RunStatus::Failed) {
                                    _status = CoordinatorStatus::Failed;
                                }
                                else {
                                    pushFinished(task);
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
    }

    void handleSubscribers() {}

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

    void pushFinished(Task *task) {
        auto lock = std::scoped_lock{_subscriberMutex};
        std::cout << "task " + task->name() + " finished" << std::endl;
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
