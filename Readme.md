Matmake2
============================

Building on Linux
------------------
### Using build script

This is the easiest way to build matmake on, and does only require you to
install a relativly new compiler.

```bash
./build-linux.sh clang++-10
```

or

```bash
./build-linux.sh g++-10
```

"clang++10" can be replaced with the compiler of your choice.
Note that you need a compiler that supports c++10 and std::filesystem.
I recommend clang++-10 or g++-10

After building, matmake will be located in the "build"-directory.


### Build using Using cmake (for matmake development)
* Install cmake, ninja and g++ (or clang++)
* Create a build forder and run cmake in it. It will probably just work

```
mkdir build
cd build
cmake .. -G Ninja
ninja
```

Windows
-------------------

Install

* Install Visual studio, clang mingw
* Then just use the cmake gui to select the right compiler 
(doing it with command line is almost impossible)


Run unit tests
-----------------------
Goto build directory.

Run `ctest`.



For more information about c++20 modules (in clang)
_____________________________________________________

https://clang.llvm.org/docs/Modules.html


## Todo
- [ ] Automatically parse visual studio debug console environment variables before build
- [ ] Include matmakefiles from other directories
- [ ] Trace command line arguments
- [ ] Shared libraries
- [ ] Static libraries
