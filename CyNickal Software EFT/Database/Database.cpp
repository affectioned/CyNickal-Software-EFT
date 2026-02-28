#include "pch.h"
#include "Database.h"
#include "TarkovDevAPI.hpp"

void Database::Initialize()
{
	if (!IsDBOnFile()) {
		std::println("[Database] EFT_Data.db not found on file!");
		CreateLocalDB();
	}

	TarkovDevAPI::FetchAll(GetTarkovDB());
}

sqlite3* Database::GetTarkovDB()
{
	if (m_TarkovDB)
		return m_TarkovDB;

	sqlite3_open("EFT_Data.db", &m_TarkovDB);

	return m_TarkovDB;
}

bool Database::IsDBOnFile()
{
	auto fs = std::filesystem::current_path();

	auto dbPath = fs / "EFT_Data.db";

	if (std::filesystem::exists(dbPath))
		return true;

	return false;
}

void Database::CreateLocalDB()
{
	std::println("[Database] Creating EFT_Data.db locally...");

	sqlite3* db = nullptr;
	if (sqlite3_open("EFT_Data.db", &db) != SQLITE_OK) {
		std::println("[Database] Failed to create database!");
		return;
	}

	const char* createItemTable = R"(
		CREATE TABLE IF NOT EXISTS item_data (
			bsg_id TEXT PRIMARY KEY,
			short_name TEXT NOT NULL,
			trader_price INTEGER
		)
	)";

	const char* createContainerTable = R"(
		CREATE TABLE IF NOT EXISTS container_data (
			bsg_id TEXT PRIMARY KEY,
			short_name TEXT NOT NULL
		)
	)";

	const char* createAmmoTable = R"(
		CREATE TABLE IF NOT EXISTS ammo_data (
			bsg_id TEXT PRIMARY KEY,
			short_name TEXT NOT NULL
		)
	)";

	const char* createExfilTable = R"(
		CREATE TABLE IF NOT EXISTS exfil_data (
			map_internal_name TEXT NOT NULL,
			exfil_display_name TEXT NOT NULL,
			pos_x REAL NOT NULL,
			pos_y REAL NOT NULL,
			pos_z REAL NOT NULL,
			PRIMARY KEY (map_internal_name, pos_x, pos_y, pos_z)
		)
	)";

	const char* createExfilIndex = R"(
		CREATE INDEX IF NOT EXISTS idx_exfil_position 
		ON exfil_data(map_internal_name, pos_x, pos_y, pos_z)
	)";

	char* errMsg = nullptr;

	if (sqlite3_exec(db, createItemTable, nullptr, nullptr, &errMsg) != SQLITE_OK ||
		sqlite3_exec(db, createContainerTable, nullptr, nullptr, &errMsg) != SQLITE_OK ||
		sqlite3_exec(db, createAmmoTable, nullptr, nullptr, &errMsg) != SQLITE_OK ||
		sqlite3_exec(db, createExfilTable, nullptr, nullptr, &errMsg) != SQLITE_OK ||
		sqlite3_exec(db, createExfilIndex, nullptr, nullptr, &errMsg) != SQLITE_OK) {

		std::println("[Database] Failed to create tables: {}", errMsg);
		sqlite3_free(errMsg);
		sqlite3_close(db);
		return;
	}

	sqlite3_close(db);
	std::println("[Database] Database created successfully!");
}