// copyright Mattias Larsson Sköld 2021

#include "coordinator.h"
#include "filesystem.h"
#include "tasklist.h"

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
