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

TEST_SUIT_END
