#include "coordinator.h"
#include "createtasks.h"
#include "filesystem.h"
#include "mls-unit-test/unittest.h"

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
    const auto json = Json::LoadFile((testPath / "matmake.json").string());

    auto matmakefile = MatmakeFile{json};

    return createTasks(matmakefile, "clang");
}

TEST_SUIT_BEGIN

TEST_CASE("0 - right status") {
    setupProject();

    auto tasks = getTasks();

    ASSERT_EQ(tasks.empty(), false);

    auto root = tasks.find("@clang");

    ASSERT_NE(root, nullptr);

    auto main = tasks.find("@main");

    ASSERT_NE(main, nullptr);

    ASSERT_EQ(main->exists(), false);

    auto settings = Settings{};
    auto coordinator = Coordinator{};
    coordinator.execute(tasks, settings);

    ASSERT_EQ(main->exists(), true);
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

        ASSERT_NE(other, nullptr);
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
