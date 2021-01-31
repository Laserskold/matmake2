# CMakeList.txt : CMake project for Matmake-v2, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.8)

project ("matmake2")

# Add source to this project's executable.
add_executable (
	matmake2
	"src/main.cpp"
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



