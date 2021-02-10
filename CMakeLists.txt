# CMakeList.txt : CMake project for Matmake-v2

cmake_minimum_required (VERSION 3.16)
if (MSVC_VERSION GREATER_EQUAL "1900")
    include(CheckCXXCompilerFlag)
    CHECK_CXX_COMPILER_FLAG("/std:c++latest" _cpp_latest_flag_supported)
    if (_cpp_latest_flag_supported)
        add_compile_options("/std:c++latest")
    endif()
else()
    set(CMAKE_CXX_STANDARD 17)
endif()

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
)

link_libraries(matmake2-core  ${CMAKE_THREAD_LIBS_INIT})

add_executable (
   matmake2
  "src/main/main.cpp"
)

include_directories(
   "${CMAKE_SOURCE_DIR}/src"
   "${CMAKE_SOURCE_DIR}/lib/json.h/include"
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

enable_testing()
add_test(NAME tests COMMAND task_test)




