
mkdir "build"
mkdir "build\gcc"

g++ -Iinclude -Ilib/json.h/include -Isrc src/*.cpp src/main/*.cpp -std=c++17 -o build/gcc/matmake.exe

echo matmake created in build/g++
