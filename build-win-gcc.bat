
mkdir "build"
mkdir "build\gcc"

g++ -Iinclude -Ilib/json.h/include -Isrc matmake.cpp -std=c++17 -o build/gcc/matmake.exe

echo matmake created in build/g++
