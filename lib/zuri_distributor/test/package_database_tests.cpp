#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <tempo_test/result_matchers.h>
#include <tempo_test/status_matchers.h>
#include <tempo_utils/file_utilities.h>
#include <zuri_distributor/package_database.h>
#include <zuri_distributor/distributor_result.h>

class PackageDatabase : public ::testing::Test {
protected:
    std::shared_ptr<zuri_distributor::PackageDatabase> packageDatabase;
    void TearDown() override {
        if (packageDatabase) {
            auto path = packageDatabase->getDatabaseFilePath();
            packageDatabase.reset();
            std::filesystem::remove(path);
        }
    }
};

TEST_F(PackageDatabase, CreateDatabaseWhenMissing)
{
    auto databaseFilePath = tempo_utils::generate_name("environment.db.XXXXXXXX");
    ASSERT_FALSE (std::filesystem::exists(databaseFilePath));

    auto createDatabaseResult = zuri_distributor::PackageDatabase::openOrCreate(databaseFilePath);
    ASSERT_THAT (createDatabaseResult, tempo_test::IsResult());
    packageDatabase = createDatabaseResult.getResult();
    ASSERT_TRUE (std::filesystem::exists(databaseFilePath));
}

TEST_F(PackageDatabase, OpenFailsWhenDatabaseDoesNotExist)
{
    auto databaseFilePath = tempo_utils::generate_name("environment.db.XXXXXXXX");
    auto createDatabaseResult = zuri_distributor::PackageDatabase::open(databaseFilePath);
    ASSERT_THAT (createDatabaseResult, tempo_test::ContainsStatus(
        zuri_distributor::DistributorCondition::kDistributorInvariant));
}
