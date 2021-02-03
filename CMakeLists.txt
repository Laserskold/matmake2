# CMakeList.txt : CMake project for Matmake-v2
cmake_minimum_required (VERSION 3.16)

if ( MSVC )
    set(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} /W4")
else()
    set(CMAKE_CXX_FLAGS
        "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wpedantic -Wno-c++20-designator")
endif()

project ("matmake2")

add_executable (
	matmake2
        "src/main.cpp"
        "src/task.cpp"
        "src/tasklist.cpp"
        "src/nativecommands.cpp"
        "src/precompiled.h"
	"src/targetnode.h"
	"src/task.h"
)

set_property(TARGET matmake2 PROPERTY CXX_STANDARD 17)

include_directories("${CMAKE_SOURCE_DIR}/lib/json.h/include")

target_precompile_headers(matmake2 PUBLIC "src/precompiled.h")

file (
    COPY
    ${CMAKE_SOURCE_DIR}/demos/
    DESTINATION
    ${CMAKE_CURRENT_BINARY_DIR}/demos
)




