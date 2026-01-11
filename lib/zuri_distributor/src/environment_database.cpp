
#include <sqlite3.h>

#include <zuri_distributor/environment_database.h>
#include <zuri_distributor/distributor_result.h>

zuri_distributor::EnvironmentDatabase::EnvironmentDatabase(
    const std::filesystem::path &databaseFilePath,
    sqlite3 *db)
    : m_databaseFilePath(databaseFilePath),
      m_db(db)
{
    TU_ASSERT (!m_databaseFilePath.empty());
    TU_ASSERT (m_db != nullptr);
}

zuri_distributor::EnvironmentDatabase::~EnvironmentDatabase()
{
    sqlite3_finalize(m_listSpecifiers);
    sqlite3_finalize(m_insertSpecifier);

    auto ret = sqlite3_close_v2(m_db);
    TU_LOG_WARN_IF (ret != SQLITE_OK) << "sqlite close failed unexpectedly: " << sqlite3_errstr(ret);
}

tempo_utils::Result<std::shared_ptr<zuri_distributor::EnvironmentDatabase>>
zuri_distributor::EnvironmentDatabase::open(const std::filesystem::path &databaseFilePath, int flags)
{
    sqlite3 *db = nullptr;
    auto ret = sqlite3_open_v2(databaseFilePath.c_str(), &db, flags, nullptr);
    if (ret != SQLITE_OK)
        return DistributorStatus::forCondition(DistributorCondition::kDistributorInvariant,
            "failed to open environment database {}: {}",
            databaseFilePath.string(), sqlite3_errstr(ret));

    auto environmentDatabase = std::shared_ptr<EnvironmentDatabase>(new EnvironmentDatabase(databaseFilePath, db));
    TU_RETURN_IF_NOT_OK (environmentDatabase->prepare());
    return environmentDatabase;
}

tempo_utils::Result<std::shared_ptr<zuri_distributor::EnvironmentDatabase>>
zuri_distributor::EnvironmentDatabase::openOrCreate(const std::filesystem::path &databaseFilePath)
{
    int flags = SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE | SQLITE_OPEN_NOMUTEX | SQLITE_OPEN_NOFOLLOW;
    return open(databaseFilePath, flags);
}

tempo_utils::Result<std::shared_ptr<zuri_distributor::EnvironmentDatabase>>
zuri_distributor::EnvironmentDatabase::open(const std::filesystem::path &databaseFilePath)
{
    int flags = SQLITE_OPEN_READWRITE | SQLITE_OPEN_NOMUTEX | SQLITE_OPEN_NOFOLLOW;
    return open(databaseFilePath, flags);
}

inline tempo_utils::Status
sqlite3_err_to_status(char *err)
{
    if (err == nullptr)
        return {};
    auto status = zuri_distributor::DistributorStatus::forCondition(
        zuri_distributor::DistributorCondition::kDistributorInvariant,
        "encountered SQL error: {}", err);
    sqlite3_free(err);
    return status;
}

tempo_utils::Status
zuri_distributor::EnvironmentDatabase::prepare()
{
    char *err = nullptr;
    const char *tail = nullptr;
    int ret;

    // exec createSpecifiersTable statement

    std::string sqlCreateSpecifiersTable = R"(
CREATE TABLE IF NOT EXISTS Specifiers (
	specifier VARCHAR(512) PRIMARY KEY,
	packageName VARCHAR(64),
	packageDomain VARCHAR(256),
	majorVersion SMALLINT,
	minorVersion SMALLINT,
	patchVersion SMALLINT
    );)";

    ret = sqlite3_exec(m_db, sqlCreateSpecifiersTable.c_str(), nullptr, nullptr, &err);
    if (ret != SQLITE_OK)
        return DistributorStatus::forCondition(DistributorCondition::kDistributorInvariant,
            "failed to create 'Specifiers' table: {}", sqlite3_errstr(ret));
    TU_RETURN_IF_NOT_OK (sqlite3_err_to_status(err));

    // exec createPackagesTable statement

    std::string sqlCreatePackagesTable = R"(
CREATE TABLE IF NOT EXISTS Packages (
	specifier VARCHAR(512) PRIMARY KEY,
	status TINYINT,
	installedAt TEXT,
    sha256 VARCHAR(64),
	FOREIGN KEY (specifier) REFERENCES Specifiers(specifier)
    );)";

    ret = sqlite3_exec(m_db, sqlCreatePackagesTable.c_str(), nullptr, nullptr, &err);
    if (ret != SQLITE_OK)
        return DistributorStatus::forCondition(DistributorCondition::kDistributorInvariant,
            "failed to create 'Packages' table: {}", sqlite3_errstr(ret));
    TU_RETURN_IF_NOT_OK (sqlite3_err_to_status(err));

    // exec createFilesTable statement

    std::string sqlCreateFilesTable = R"(
CREATE TABLE IF NOT EXISTS Files (
    specifier VARCHAR(512),
    path VARCHAR(1024),
    sha256 VARCHAR(64),
	FOREIGN KEY (specifier) REFERENCES Specifiers(specifier),
    PRIMARY KEY (specifier, path)
    );)";

    ret = sqlite3_exec(m_db, sqlCreateFilesTable.c_str(), nullptr, nullptr, &err);
    if (ret != SQLITE_OK)
        return DistributorStatus::forCondition(DistributorCondition::kDistributorInvariant,
            "failed to create 'Files' table: {}", sqlite3_errstr(ret));
    TU_RETURN_IF_NOT_OK (sqlite3_err_to_status(err));

    // prepare listSpecifiers statement

    std::string sqlListSpecifiers = R"(
SELECT specifier FROM Specifiers;)";

    ret = sqlite3_prepare_v3(m_db, sqlListSpecifiers.c_str(), sqlListSpecifiers.size(),
        0, &m_listSpecifiers, &tail);
    if (ret != SQLITE_OK)
        return DistributorStatus::forCondition(DistributorCondition::kDistributorInvariant,
            "failed to prepare listSpecifiers statement: {}", sqlite3_errstr(ret));
    if (tail != nullptr && std::strlen(tail) > 0)
        return DistributorStatus::forCondition(DistributorCondition::kDistributorInvariant,
            "encountered unexpected trailing data preparing listSpecifiers statement: '{}'", tail);

    // prepare insertSpecifier statement

    std::string sqlInsertSpecifier = R"(
INSERT INTO Specifiers
    (specifier, packageName, packageDomain, majorVersion, minorVersion, patchVersion)
    VALUES (?, ?, ?, ?, ?, ?);)";

    ret = sqlite3_prepare_v3(m_db, sqlInsertSpecifier.c_str(), sqlInsertSpecifier.size(),
        0, &m_insertSpecifier, &tail);
    if (ret != SQLITE_OK)
        return DistributorStatus::forCondition(DistributorCondition::kDistributorInvariant,
            "failed to prepare insertSpecifier statement: {}", sqlite3_errstr(ret));
    if (tail != nullptr && std::strlen(tail) > 0)
        return DistributorStatus::forCondition(DistributorCondition::kDistributorInvariant,
            "encountered unexpected trailing data preparing insertSpecifier statement: '{}'", tail);

    return {};
}

std::filesystem::path
zuri_distributor::EnvironmentDatabase::getDatabaseFilePath() const
{
    return m_databaseFilePath;
}
