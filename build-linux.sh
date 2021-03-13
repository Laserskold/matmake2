mkdir -p build

CXX=$1

echo building with ${CXX}...

${CXX} -o build/matmake2 matmake.cpp \
        -Isrc -Ilib/json.h/include -std=c++17 -pthread -g

echo done...
