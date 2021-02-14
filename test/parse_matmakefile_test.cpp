#define DO_NOT_CATCH_ERRORS

#include "line.h"
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
        ASSERT_EQ(line.value.size(), 1);
        ASSERT_EQ(line.value.front(), "there");
    }

    {
        auto ss = std::istringstream{"main\n  in = \n    x.cpp\n"};

        auto line1 = Line{ss};
        ASSERT_EQ(line1.name, "main");
        ASSERT_EQ(line1.type, Line::Normal);
        ASSERT_EQ(line1.indent, 0);

        auto line2 = Line{ss};
        ASSERT_EQ(line2.name, "in");
        ASSERT_EQ(line2.type, Line::UnfinishedAssignment);
        ASSERT_EQ(line2.indent, 2);

        auto line3 = Line{ss};
        ASSERT_EQ(line3.name, "x.cpp");
        ASSERT_EQ(line3.type, Line::Normal);
        ASSERT_EQ(line3.indent, 4);
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

    ASSERT_EQ(first["name"].string(), "main");
    ASSERT_EQ(first.type, Json::Object);
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

    ASSERT_EQ(first["name"].string(), "main");
    ASSERT_EQ(first.type, Json::Object);
    ASSERT_EQ(first["out"].string(), "main");

    ASSERT_EQ(first["in"].size(), 2);
    ASSERT_EQ(first["in"].type, Json::Array);
    ASSERT_EQ(first["in"].front().string(), "src/*.cpp");
    ASSERT_EQ(first["in"].back().string(), "src/*.cppm");
}

TEST_SUIT_END
