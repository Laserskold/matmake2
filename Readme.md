Matmake2
============================

Matmake2 is a robust, fast and small c++-module-supporting build system.

Features:
* Small configuration files
* Automatic dependency checking
* c++-20 modules support

Example syntax
----------------

The following will include all cpp and cppm-files in "src" and
build a executable named `main` or `main.exe`  (depending on platform)

```make

main
   src = *.cpp *.cppm
   out = main
   config = c++20 modules
    
all:
  in = @main

```


Example of more complicated file:

```make

main
  src =
    *.cpp
    *.cppm
  out = main       # On windows ".exe" will be added to this name
   
  

gcc
  in = @main       # Specify that when building "gcc" main will be built
  config =
    c++17
    modules
    Wall
    Wpedantic
    debug          # Compile with debug information
  flagstyle = gcc  # gcc or msvc: how the flags look. Like "-Iinclude" or like "/Iinclude"
    
  

```

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

### Build for windows

#### G++
Install Mingw

run

```bat
build-win-gcc.bat
```

#### Visual studio 2019 Community Edition
Install visual studio 2019
run 

```bat
build-win-msvc.bat
```

#### Clang (requires Visual studio 2019 to be installed)

run
```
build-win-clang.bat
```


For more information about c++20 modules (in clang)
---------------------------------------------------

https://clang.llvm.org/docs/Modules.html


## Todo
- [ ] Automatically parse visual studio debug console environment variables before build
- [ ] Include matmakefiles from other directories
- [ ] Trace command line arguments
- [ ] Shared libraries
- [ ] Static libraries
