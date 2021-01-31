// copyright Mattias Larsson Sköld 2021

#include "json/json.h"
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>

int main(int, char **) {

    std::cout << "hello\n" << std::endl;

    auto json = Json{};
    auto file = std::ifstream{"demos/project1/tasks.json"};
    if (file.is_open()) {
        file >> json;
    }
    else {
        std::cerr << "could not open file\n";
    }

    std::cout << json;
    return 0;
}
