
setlocal

mkdir "build"

g++ -Iinclude -Ilib/json.h/include -Isrc matmake.cpp -std=c++17 -o build/matmake2.exe

echo matmake created in build
