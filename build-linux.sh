mkdir -p build

CXX=$1

${CXX} -o build/matmake src/*.cpp src/main/main.cpp \
        -Isrc -Ilib/json.h/include -std=c++17 -pthread
