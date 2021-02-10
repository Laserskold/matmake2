module;

#include <string>

export module other;

import other2;

// Compile with commands
// clang++-11 -std=c++2a -fmodules-ts -c src/test.pcm  -o src/test.o

export int other() {
    auto x = std::string{"hello"};
    return 10 + other2() + x.size();
}

export std::string str() {
    return "hello there";
}
