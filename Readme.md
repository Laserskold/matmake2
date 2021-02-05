Matmake2
============================

Linux
------------------

### Using cmake
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

* Install Visual studio, clang mingw
* Then just use the cmake gui to select the right compiler 
(doing it with command line is almost impossible)



Run unit tests
-----------------------
Goto build directory.

Run `ctest`.
