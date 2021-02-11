#include "mls-unit-test/unittest.h"
#include "parsematmakefile.h"

using namespace std::literals;

TEST_SUIT_BEGIN

TEST_CASE("strip()") {
    auto x = "   hello   "s;

    auto y = strip(x);

    ASSERT_EQ(y, "hello");
}

TEST_CASE("Line") {
    {
        auto line = Line{"   hello there"};

        ASSERT_EQ(line.type, Line::Normal);
        ASSERT_EQ(line.indent, 3);
        ASSERT_EQ(line.name, "hello there");
    }

    {

        auto line = Line{"  hello = there"};

        ASSERT_EQ(line.type, Line::Assignment);
        ASSERT_EQ(line.indent, 2);
        ASSERT_EQ(line.name, "hello");
        ASSERT_EQ(line.value, "there");
    }
}

TEST_CASE("Minimal file") {
    auto content = std::istringstream{R"_(

# comment
main
  in = src/*.cpp
  out = main
)_"};

    const auto json = parseMatmakefile(content);

    ASSERT_EQ(json.size(), 1);

    auto &first = json.front();

    ASSERT_EQ(first["in"].string(), "src/*.cpp");
    ASSERT_EQ(first["out"].string(), "main");
}

TEST_CASE("Multiline value") {
    auto content = std::istringstream{R"_(

# comment
main
  in =
    src/*.cpp
    src/*.cppm
  out = main
)_"};

    const auto json = parseMatmakefile(content);

    ASSERT_EQ(json.size(), 1);

    auto &first = json.front();

    ASSERT_EQ(first["in"].string(), "src/*.cpp");
    ASSERT_EQ(first["out"].string(), "main");
}

TEST_SUIT_END
