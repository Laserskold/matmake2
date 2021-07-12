#include "execute.h"
#include "os.h"

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
