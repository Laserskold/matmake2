#include "defaultfile.h"

namespace {

const char *defaultCompilerSource = R"_(
[
  {
    "name": "clang",
    "in": [ "@all" ],
    "cxx": "clang++-11",
    "dir": "build/clang",
    "flags": [ "-fmodules-ts", "-std=c++2a" ],
    "command": "[root]",
    "includeprefix": "-I",
    "commands": {
      "cxx": "{c++} -x c++ {src} {modules} -o {out} -c -MD -MF {depfile} {cxxflags} {flags} {includes}",
      "exe": "{c++} {in} -o {out} {ldflags} {includes}",
      "gch": "{c++} {in} -o {out} -MD -MF {depfile} {cxxflags} {flags} {includes}",
      "eem": "{c++} {in} {includes} -E > {out}",
      "pcm": "{c++} {cxxflags} {flags} {includes} {modules} --precompile -x c++-module {src} -o {out} ",
      "cxxm": "{c++} {cxxflags} {flags} {includes} -c {in} -o {out} "
    }
  },
  {
    "name": "gcc",
    "in": [ "@all" ],
    "cxx": "g++",
    "dir": "build/clang",
    "flags": [ "-std=c++2a" ],
    "command": "[root]",
    "includeprefix": "-I",
    "commands": {
      "cxx": "{c++} -x c++ {src} {modules} -o {out} -c -MD -MF {depfile} {cxxflags} {flags} {includes}",
      "exe": "{c++} {in} -o {out} {ldflags} {includes}",
      "gch": "{c++} {in} -o {out} -MD -MF {depfile} {cxxflags} {flags} {includes}",
      "eem": "{c++} {in} {includes} -E > {out}",
      "pcm": "{c++} {cxxflags} {flags} {includes} {modules} --precompile -x c++-module {src} -o {out} ",
      "cxxm": "{c++} {cxxflags} {flags} {includes} -c {in} -o {out} "
    }
  }
]
)_";

}

Json defaultCompiler() {
    return Json{}.parse(defaultCompilerSource);
}
