#include "exampleproject.h"
#include <fstream>
#include <iostream>

namespace {

const char *exampleMatmakefile = R"_(
# using matmake2: https://github.com/laserskold/matmake2

main
  out = main
  src =
    src/*.cpp
  config =
    c++17
    Wall
  command = [exe]

all
  in = @main
  includes =
    include

)_";

const char *exampleMain = R"_(

#include <iostream>

int main(int argc, char ** argv) {
    std::cout << "hello from " << argv[0] << std::endl;
}

)_";

const char *exampleClangFormat = R"_(

Language: Cpp
BasedOnStyle: LLVM
IndentWidth: 4
SortIncludes: true
AccessModifierOffset: -4
AlwaysBreakTemplateDeclarations: true
AllowShortFunctionsOnASingleLine: Empty
BinPackArguments: false
BinPackParameters: false
BreakBeforeBraces: Custom
BraceWrapping:
  BeforeCatch: true
  BeforeElse: true

AlwaysBreakAfterReturnType: None
PenaltyReturnTypeOnItsOwnLine: 1000000
BreakConstructorInitializers: BeforeComma

)_";

} // namespace

void createExampleProject(filesystem::path path) {
    path = filesystem::absolute(path);
    std::cout << "creating project in " << path << "..." << std::endl;

    if (!filesystem::exists(path.parent_path())) {
        std::cerr << "cannot create project in nonexisting directory"
                  << path.parent_path() << std::endl;
        return;
    }

    if (filesystem::exists(path / "Matmakefile") ||
        filesystem::exists(path / "src" / "main.cpp")) {
        std::cerr << "There is already files in that folder: exiting...\n";
        return;
    }

    filesystem::create_directories(path / "src");
    filesystem::create_directories(path / "include");

    std::ofstream{path / "Matmakefile"} << exampleMatmakefile;
    std::ofstream{path / "src" / "main.cpp"} << exampleMain;

    auto gitignorePath = path / ".gitignore";
    std::ofstream{gitignorePath} << "build/\n";

    auto clangFormatPath = path / ".clangformat";
    std::ofstream{clangFormatPath} << exampleClangFormat;
}
