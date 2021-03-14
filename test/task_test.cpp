#include "mls-unit-test/unittest.h"
#include "task.h"

TEST_SUIT_BEGIN

TEST_CASE("isModule") {
    auto task = Task{};

    task.out("test.pcm");

    EXPECT_EQ(task.isModule(), true);
}

TEST_CASE("property: modules") {
    auto dep = Task{};
    dep.out("other.pcm");

    auto src = Task{};
    src.out("main.cpp");

    auto main = Task{};
    main.out("main.pcm");

    main.pushIn(&dep);
    main.pushIn(&src);

    EXPECT_NE(main.property("modules").find("other.pcm"), std::string::npos);
    EXPECT_EQ(main.property("modules").find("main.cpp"), std::string::npos);
}

TEST_CASE("property: config") {
    auto src = Task{};

    auto main = Task{};
    main.pushIn(&src);

    main.config({"c++11"});

    EXPECT_EQ(src.property("standard"), "-std=c++11");
    EXPECT_EQ(src.property("flags"), "-std=c++11");

    main.flagStyle("msvc");

    EXPECT_EQ(src.property("standard"), "/std:c++11");
    EXPECT_EQ(src.property("flags"), "/std:c++11");
}

TEST_CASE("property: compiler") {
    auto task = Task{};

    task.cc("c-compiler");
    EXPECT_EQ(task.cc(), "c-compiler");
    EXPECT_EQ(task.property("cc"), "c-compiler");

    task.cxx("c++-compiler");
    EXPECT_EQ(task.cxx(), "c++-compiler");
    EXPECT_EQ(task.property("c++"), "c++-compiler");
}

TEST_CASE("property: build-location") {
    auto task = Task{};

    task.dir(BuildLocation::Real, "real");
    task.dir(BuildLocation::Intermediate, "obj");

    auto subtask = Task{};
    subtask.dir(BuildLocation::Real, "gcc");

    task.pushIn(&subtask);

    EXPECT_EQ(subtask.dir(), "real/gcc");

    subtask.buildLocation(BuildLocation::Intermediate);

    EXPECT_EQ(subtask.dir(), "obj/gcc");
}

TEST_SUIT_END
