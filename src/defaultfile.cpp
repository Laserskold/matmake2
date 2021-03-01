#include "defaultfile.h"

namespace {

const char *defaultCompilerSource = R"_(
[
  {
    "name": "clang",
    "in": [ "@all" ],
    "cxx": "clang++-11",
    "ar": "ar",
    "dir": "build/clang",
    "command": "[root]",
    "includeprefix": "-I",
    "commands": {
      "cxx": "{c++} -x c++ {src} {modules} -o {out} -c {cxxflags} {flags} {includes}",
      "cc": "{c++} -x c {src} -o {out} -c {cxxflags} {flags} {includes}",
      "exe": "{c++} {in} -o {out} {ldflags} {flags} {includes}",
      "gch": "{c++} {in} -o {out} {depfile} {cxxflags} {flags} {includes}",
      "eem": "{c++} {in} {standard} {includes} {eflags} -E > {out}",
      "copy": "cp {in} {out}",
      "pcm": "{c++} {cxxflags} {flags} {includes} {modules} --precompile -x c++-module {src} -o {out} ",
      "cxxm": "{c++} {cxxflags} {flags} {includes} -c {in} -o {out} ",
      "static": "{ar} -rs {out} {in}"
    }
  },
  {
    "name": "gcc",
    "in": [ "@all" ],
    "cxx": "g++",
    "ar": "ar",
    "dir": "build/gcc",
    "flags": [ "-std=c++17" ],
    "command": "[root]",
    "includeprefix": "-I",
    "commands": {
      "cxx": "{c++} -x c++ {src} {modules} -o {out} -c {cxxflags} {flags} {includes}",
      "cc": "{c++} -x c {src} {modules} -o {out} -c {cxxflags} {flags} {includes}",
      "exe": "{c++} {in} -o {out} {ldflags} {flags} {includes}",
      "gch": "{c++} {in} -o {out} {depfile} {cxxflags} {flags} {includes}",
      "eem": "{c++} {in} {standard} {includes} {eflags} -E > {out}",
      "copy": "cp {in} {out}",
      "pcm": "{c++} {cxxflags} {flags} {includes} {modules} --precompile -x c++-module {src} -o {out} ",
      "cxxm": "{c++} {cxxflags} {flags} {includes} -c {in} -o {out} ",
      "static": "{ar} -rs {out} {in}"
    }
  }
]
)_";

}

Json defaultCompiler() {
    return Json{}.parse(defaultCompilerSource);
}
