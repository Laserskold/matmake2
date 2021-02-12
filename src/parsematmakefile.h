
#pragma once
#include "matmakefile.h"

#include "json/json.h"
#include <algorithm>
#include <iostream>

Json parseMatmakefile(std::istream &file);

Json parseMatmakefile(filesystem::path path);
