#include "coordinator.h"
#include "createtasks.h"
#include "filesystem.h"
#include "mls-unit-test/unittest.h"
#include "parsematmakefile.h"

const auto workingDirectory = filesystem::current_path();

const auto source = workingDirectory / "demos" / "modules";

const auto testPath = workingDirectory / "sandbox" / "test";

void setupProject() {
    filesystem::remove_all(testPath);
    filesystem::current_path(workingDirectory);
    filesystem::create_directories(testPath);
    filesystem::copy(source, testPath, filesystem::copy_options::recursive);
    filesystem::current_path(testPath);
}

auto getTasks() {
    auto json = parseMatmakefile((testPath / "Matmakefile").string());

    auto matmakefile = MatmakeFile{json};

    return createTasks(matmakefile, "clang");
}

TEST_SUIT_BEGIN

TEST_CASE("0 - right status") {
    setupProject();

    auto tasks = getTasks();

    ASSERT_EQ(tasks.empty(), false);

    auto root = tasks.find("@clang");

    EXPECT_TRUE(root);

    auto main = tasks.find("@main");

    EXPECT_TRUE(main);

    EXPECT_FALSE(main->exists());

    auto settings = Settings{};
    auto coordinator = Coordinator{};
    coordinator.execute(tasks, settings);

    EXPECT_TRUE(main->exists());
}

TEST_CASE("1 - touch file -> rebuild") {
    setupProject();

    filesystem::file_time_type time1;

    {
        auto tasks = getTasks();

        ASSERT_EQ(tasks.empty(), false);

        auto settings = Settings();
        auto coordinator = Coordinator{};
        coordinator.execute(tasks, settings);

        auto main = tasks.find("@main");
        time1 = filesystem::last_write_time(main->out());

        auto other = tasks.find("./src/other.cppm");

        EXPECT_TRUE(other);
        std::ofstream{other->out(), std::ios::app} << " "; // Touch file
    }

    {

        auto tasks = getTasks();

        ASSERT_EQ(tasks.empty(), false);

        auto settings = Settings();
        auto coordinator = Coordinator{};
        coordinator.execute(tasks, settings);

        auto main = tasks.find("@main");

        filesystem::file_time_type time2 =
            filesystem::last_write_time(main->out());

        bool isGt = time2 > time1;
        ASSERT_EQ(isGt, true);
    }
}

TEST_SUIT_END
