#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <tempo_test/result_matchers.h>
#include <tempo_test/status_matchers.h>
#include <tempo_utils/file_utilities.h>
#include <zuri_distributor/environment_database.h>
#include <zuri_distributor/distributor_result.h>

class EnvironmentDatabase : public ::testing::Test {
protected:
    std::shared_ptr<zuri_distributor::EnvironmentDatabase> environmentDatabase;
    void TearDown() override {
        if (environmentDatabase) {
            auto path = environmentDatabase->getDatabaseFilePath();
            environmentDatabase.reset();
            std::filesystem::remove(path);
        }
    }
};

TEST_F(EnvironmentDatabase, CreateDatabaseWhenMissing)
{
    auto databaseFilePath = tempo_utils::generate_name("environment.db.XXXXXXXX");
    ASSERT_FALSE (std::filesystem::exists(databaseFilePath));

    auto createDatabaseResult = zuri_distributor::EnvironmentDatabase::openOrCreate(databaseFilePath);
    ASSERT_THAT (createDatabaseResult, tempo_test::IsResult());
    environmentDatabase = createDatabaseResult.getResult();
    ASSERT_TRUE (std::filesystem::exists(databaseFilePath));
}

TEST_F(EnvironmentDatabase, OpenFailsWhenDatabaseDoesNotExist)
{
    auto databaseFilePath = tempo_utils::generate_name("environment.db.XXXXXXXX");
    auto createDatabaseResult = zuri_distributor::EnvironmentDatabase::open(databaseFilePath);
    ASSERT_THAT (createDatabaseResult, tempo_test::ContainsStatus(
        zuri_distributor::DistributorCondition::kDistributorInvariant));
}
