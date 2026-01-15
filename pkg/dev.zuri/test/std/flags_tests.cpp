#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <lyric_test/lyric_tester.h>
#include <lyric_test/matchers.h>
#include <tempo_config/workspace_config.h>
#include <tempo_test/tempo_test.h>
#include <zuri_test/zuri_tester.h>

#include "test_utils.h"

class StdFlagsFlags : public ::testing::Test {
protected:
    std::unique_ptr<zuri_test::ZuriTester> tester;

    void SetUp() override {
        auto runtime = get_global_test_runtime();
        zuri_test::TesterOptions options;
        options.localPackages.emplace_back(ZURI_STD_PACKAGE_PATH);
        tester = std::make_unique<zuri_test::ZuriTester>(runtime, options);
        TU_RAISE_IF_NOT_OK (tester->configure());
    }
};

TEST_F(StdFlagsFlags, EvaluateSetAndCheckFlags)
{
    auto result = tester->runModule(R"(
        import from "dev.zuri.pkg://std-0.0.1@zuri.dev/flags" ...

        defenum Colors {
            val Value: Int
            init(value: Int) {
                set this.Value = value
            }
            case Red(1)
            case Yellow(2)
            case Blue(4)
        }

        definstance ColorsInstance {
            impl IntoFlags[Colors] {
                def ToValue(color: Colors): Int {
                    color.Value
                }
            }
        }
        using ColorsInstance

        val flags: Flags[Colors] = Flags[Colors]{}
        flags.Set(Red)
        flags.Contains(Red)
    )");

    ASSERT_THAT (result, tempo_test::ContainsResult(RunModule(
        DataCellBool(true))));
}
