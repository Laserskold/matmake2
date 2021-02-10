module;

#include <string>

export module other;

import other2;

// Compile with commands
// clang++-11 -std=c++2a -fmodules-ts -c src/test.pcm  -o src/test.o

export int other() {
    return 10 + other2();
}
