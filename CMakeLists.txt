# CMakeList.txt : CMake project for Matmake-v2
cmake_minimum_required (VERSION 3.16)


if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    set(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -time-trace")
endif()

project ("matmake2" CXX)

find_package( Threads )

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


target_link_libraries( matmake2 ${CMAKE_THREAD_LIBS_INIT} )

set_property(TARGET matmake2 PROPERTY CXX_STANDARD 17)

include_directories("${CMAKE_SOURCE_DIR}/lib/json.h/include")

target_precompile_headers(matmake2 PUBLIC "src/precompiled.h")

file (
    COPY
    ${CMAKE_SOURCE_DIR}/demos/
    DESTINATION
    ${CMAKE_CURRENT_BINARY_DIR}/demos
)




