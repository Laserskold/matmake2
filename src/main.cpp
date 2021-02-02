// copyright Mattias Larsson Sköld 2021

#include "filesystem.h"
#include "tasklist.h"
#include <map>
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

        //        auto nThreads = std::thread::hardware_concurrency();
        auto nThreads = 1;

        {
            auto lock = std::scoped_lock{_todoMutex};
            for (auto &task : tasks) {
                auto state = task.state();
                if (state == TaskState::DirtyReady) {
                    _todo.push_back(&task);
                    ++_remainingTasks;
                }
                else if (state == TaskState::DirtyWaiting) {
                    ++_remainingTasks;
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
                    if (auto task = getTask()) {
                        auto command =
                            ProcessedCommand{task->command()}.expand(*task);

                        if (!command.empty()) {
                            if (run(command) == RunStatus::Failed) {
                                _status = CoordinatorStatus::Failed;
                            }
                        }
                    }
                    else {
                        std::this_thread::sleep_for(10ms);
                    }
                }
            });
        }

        for (auto &worker : workers) {
            worker.join();
        }
    }

    Task *getTask() {
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

private:
    std::vector<std::thread> workers;
    CoordinatorStatus _status = CoordinatorStatus::NotStarted;

    // Give the threads something to do
    std::mutex _todoMutex;
    std::vector<Task *> _todo;

    // Handle when tasks are finished
    size_t _remainingTasks;
    std::mutex _subscriberMutex;
    std::vector<Task *> _finished;
};

int main(int, char **) {
    filesystem::current_path("demos/project1");

    auto tasks = parseTasks("tasks.json");

    printFlat(*tasks);

    std::cout << "\ntreeview\n==================\n";
    auto &root = *tasks->find("@g++");
    root.print();

    std::cout << "building... \n";

    auto coordinator = Coordinator{};

    coordinator.execute(*tasks);

    return 0;
}
