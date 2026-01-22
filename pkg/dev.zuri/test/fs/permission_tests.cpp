#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <lyric_test/lyric_tester.h>
#include <lyric_test/matchers.h>
#include <tempo_test/tempo_test.h>
#include <zuri_test/zuri_tester.h>

#include "test_utils.h"

class FsPermission : public ::testing::Test {
protected:
    std::unique_ptr<zuri_test::ZuriTester> tester;

    void SetUp() override {
        auto runtime = get_global_test_runtime();
        zuri_test::TesterOptions options;
        options.localPackages.emplace_back(ZURI_STD_PACKAGE_PATH);
        options.localPackages.emplace_back(ZURI_FS_PACKAGE_PATH);
        tester = std::make_unique<zuri_test::ZuriTester>(runtime, options);
        TU_RAISE_IF_NOT_OK (tester->configure());
    }
};

TEST_F(FsPermission, EvaluateSetAndCheck)
{
    auto result = tester->runModule(R"(
        import from "dev.zuri.pkg://fs-0.0.1@zuri.dev/permission" ...

        val perms: Permissions = Permissions{UserRead, UserWrite}
        perms.Mode.Contains(UserRead)
    )");

    ASSERT_THAT (result, tempo_test::ContainsResult(RunModule(
        DataCellBool(true))));
}
