#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <lyric_test/lyric_tester.h>
#include <lyric_test/matchers.h>
#include <tempo_config/workspace_config.h>
#include <tempo_test/tempo_test.h>
#include <zuri_test/zuri_tester.h>

class FsAccess : public ::testing::Test {
protected:
    std::unique_ptr<zuri_test::ZuriTester> tester;

    void SetUp() override {
        zuri_test::TesterOptions options;
        options.localPackages.emplace_back(ZURI_STD_PACKAGE_PATH);
        options.localPackages.emplace_back(ZURI_FS_PACKAGE_PATH);
        tester = std::make_unique<zuri_test::ZuriTester>(options);
        TU_RAISE_IF_NOT_OK (tester->configure());
    }
};

TEST_F(FsAccess, EvaluateSetAndCheckFlags)
{
    auto result = tester->runModule(R"(
        import from "dev.zuri.pkg://fs-0.0.1@zuri.dev/access" ...

        val access: AccessMode = AccessMode{UserRead, UserWrite}
        access.Mode.Contains(UserRead)
    )");

    ASSERT_THAT (result, tempo_test::ContainsResult(RunModule(
        DataCellBool(true))));
}
