
setlocal

mkdir "build"

call "C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build\vcvarsall.bat" x86_amd64

clang++.exe -Iinclude -Ilib/json.h/include -Isrc matmake.cpp -std=c++17 -o build/matmake2.exe

echo matmake created in build
