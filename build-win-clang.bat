mkdir "build"
mkdir "build\clang"

call "C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build\vcvarsall.bat" x86_amd64

clang++.exe -Iinclude -Ilib/json.h/include -Isrc src/*.cpp src/main/*.cpp -std=c++17 -o build/clang/matmake.exe

echo matmake created in build/clang
