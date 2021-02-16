# CMakeList.txt : CMake project for Matmake-v2

cmake_minimum_required (VERSION 3.16)

set(CMAKE_CXX_STANDARD 17)

if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    set(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -time-trace")
endif()

project ("matmake2" CXX)

find_package( Threads )

add_library(
   matmake2-core
   STATIC
   "src/task.cpp"
   "src/tasklist.cpp"
   "src/nativecommands.cpp"
   "src/precompiled.h"
   "src/targetnode.h"
   "src/task.h"
   "src/settings.cpp"
   "src/settings.h"
   "src/matmakefile.cpp"
   "src/defaultfile.cpp"
   "src/defaultfile.h"
   "src/parsematmakefile.cpp"
   "src/translateconfig.cpp"
)

add_subdirectory(lib/json.h)

target_link_libraries(matmake2-core PUBLIC json)

link_libraries(matmake2-core  ${CMAKE_THREAD_LIBS_INIT} )

add_executable (
   matmake2
  "src/main/main.cpp"
)

include_directories(
   "${CMAKE_SOURCE_DIR}/src"
   )

target_precompile_headers(matmake2-core PUBLIC "src/precompiled.h")

target_precompile_headers(matmake2 REUSE_FROM matmake2-core)

file (
    COPY
    ${CMAKE_SOURCE_DIR}/demos/
    DESTINATION
    ${CMAKE_CURRENT_BINARY_DIR}/demos
)

## ===== Tests =====================================

add_executable (task_test test/task_test.cpp)
add_executable (build_test test/build_test.cpp)
add_executable (parse_matmakefile_test test/parse_matmakefile_test.cpp)

target_precompile_headers(task_test REUSE_FROM matmake2-core)
target_precompile_headers(build_test REUSE_FROM matmake2-core)
target_precompile_headers(parse_matmakefile_test REUSE_FROM matmake2-core)

enable_testing()
add_test(NAME task_test COMMAND task_test)
add_test(NAME parse_matmakefile_test COMMAND parse_matmakefile_test)

if (WIN32)
else()
add_test(NAME build_test COMMAND build_test)
endif()




