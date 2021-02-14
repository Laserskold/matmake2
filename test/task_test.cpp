#include "mls-unit-test/unittest.h"
#include "task.h"

TEST_SUIT_BEGIN

TEST_CASE("isModule") {
    auto task = Task{};

    task.out("test.pcm");

    ASSERT_EQ(task.isModule(), true);
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

    ASSERT_NE(main.property("modules").find("other.pcm"), std::string::npos);
    ASSERT_EQ(main.property("modules").find("main.cpp"), std::string::npos);
}

TEST_CASE("property: config") {
    auto src = Task{};

    auto main = Task{};
    main.pushIn(&src);

    main.config({"c++11"});

    ASSERT_EQ(src.property("standard"), "-std=c++11");
    ASSERT_EQ(src.property("flags"), "-std=c++11");

    main.flagStyle("msvc");

    ASSERT_EQ(src.property("standard"), "/std:c++11");
    ASSERT_EQ(src.property("flags"), "/std:c++11");
}

TEST_SUIT_END
