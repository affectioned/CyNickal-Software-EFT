#pragma once

class TarkovDevAPI
{
public:
	static void FetchAll(sqlite3* db);

private:
	static constexpr const char* API_URL = "https://api.tarkov.dev/graphql";
	static constexpr int64_t FetchIntervalSeconds = 60 * 60 * 24; // 24 hours

	static void FetchItems(sqlite3* db);
	static void FetchContainers(sqlite3* db);
	static void FetchAmmo(sqlite3* db);
	static void FetchExfils(sqlite3* db);

	static int64_t GetFileAgeSecs();
};
