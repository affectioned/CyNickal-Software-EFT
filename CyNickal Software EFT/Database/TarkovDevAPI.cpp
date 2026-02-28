#include "pch.h"
#include "TarkovDevAPI.hpp"
#include "Network/Classes/CGraphQLRequest/CGraphQLRequest.hpp"

void TarkovDevAPI::FetchAll(sqlite3* db)
{
	std::println("[TarkovDevAPI] Fetching latest data from tarkov.dev...");
	FetchItems(db);
	FetchContainers(db);
	FetchAmmo(db);
	FetchExfils(db);
	std::println("[TarkovDevAPI] Fetch complete.");
}

void TarkovDevAPI::FetchItems(sqlite3* db)
{
	constexpr const char* query = R"(
	{
	  items(lang: en) {
	    id
	    sellFor {
	      priceRUB
	      vendor {
	        name
	      }
	    }
	    shortName
	  }
	}
	)";

	CGraphQLRequest req(API_URL, query);
	if (req.GetResponse().empty())
	{
		std::println("[TarkovDevAPI] FetchItems: empty response, skipping.");
		return;
	}

	auto json = nlohmann::json::parse(req.GetResponse(), nullptr, false);
	if (json.is_discarded() || !json.contains("data") || !json["data"].contains("items"))
	{
		std::println("[TarkovDevAPI] FetchItems: failed to parse response.");
		return;
	}

	sqlite3_exec(db, "BEGIN TRANSACTION;", nullptr, nullptr, nullptr);

	sqlite3_stmt* stmt = nullptr;
	sqlite3_prepare_v2(db,
		"INSERT OR IGNORE INTO item_data (bsg_id, short_name, trader_price) VALUES (?, ?, ?);",
		-1, &stmt, nullptr);

	for (const auto& item : json["data"]["items"])
	{
		int highestTraderPrice = -1;
		for (const auto& sell : item["sellFor"])
		{
			std::string vendor = sell["vendor"]["name"].get<std::string>();
			if (vendor == "Flea Market")
				continue;
			int price = sell["priceRUB"].get<int>();
			if (price > highestTraderPrice)
				highestTraderPrice = price;
		}

		std::string id = item["id"].get<std::string>();
		std::string shortName = item["shortName"].get<std::string>();

		sqlite3_bind_text(stmt, 1, id.c_str(), -1, SQLITE_TRANSIENT);
		sqlite3_bind_text(stmt, 2, shortName.c_str(), -1, SQLITE_TRANSIENT);
		sqlite3_bind_int(stmt, 3, highestTraderPrice);
		sqlite3_step(stmt);
		sqlite3_reset(stmt);
	}

	sqlite3_finalize(stmt);
	sqlite3_exec(db, "COMMIT;", nullptr, nullptr, nullptr);
	std::println("[TarkovDevAPI] FetchItems: done.");
}

void TarkovDevAPI::FetchContainers(sqlite3* db)
{
	constexpr const char* query = R"(
	{
	  lootContainers {
	    id
	    name
	  }
	}
	)";

	CGraphQLRequest req(API_URL, query);
	if (req.GetResponse().empty())
	{
		std::println("[TarkovDevAPI] FetchContainers: empty response, skipping.");
		return;
	}

	auto json = nlohmann::json::parse(req.GetResponse(), nullptr, false);
	if (json.is_discarded() || !json.contains("data") || !json["data"].contains("lootContainers"))
	{
		std::println("[TarkovDevAPI] FetchContainers: failed to parse response.");
		return;
	}

	sqlite3_exec(db, "BEGIN TRANSACTION;", nullptr, nullptr, nullptr);

	sqlite3_stmt* stmt = nullptr;
	sqlite3_prepare_v2(db,
		"INSERT OR IGNORE INTO container_data (bsg_id, short_name) VALUES (?, ?);",
		-1, &stmt, nullptr);

	for (const auto& container : json["data"]["lootContainers"])
	{
		std::string id = container["id"].get<std::string>();
		std::string name = container["name"].get<std::string>();

		sqlite3_bind_text(stmt, 1, id.c_str(), -1, SQLITE_TRANSIENT);
		sqlite3_bind_text(stmt, 2, name.c_str(), -1, SQLITE_TRANSIENT);
		sqlite3_step(stmt);
		sqlite3_reset(stmt);
	}

	sqlite3_finalize(stmt);
	sqlite3_exec(db, "COMMIT;", nullptr, nullptr, nullptr);
	std::println("[TarkovDevAPI] FetchContainers: done.");
}

void TarkovDevAPI::FetchAmmo(sqlite3* db)
{
	constexpr const char* query = R"(
	{
	  ammo {
	    item {
	      id
	      shortName
	    }
	  }
	}
	)";

	CGraphQLRequest req(API_URL, query);
	if (req.GetResponse().empty())
	{
		std::println("[TarkovDevAPI] FetchAmmo: empty response, skipping.");
		return;
	}

	auto json = nlohmann::json::parse(req.GetResponse(), nullptr, false);
	if (json.is_discarded() || !json.contains("data") || !json["data"].contains("ammo"))
	{
		std::println("[TarkovDevAPI] FetchAmmo: failed to parse response.");
		return;
	}

	sqlite3_exec(db, "BEGIN TRANSACTION;", nullptr, nullptr, nullptr);

	sqlite3_stmt* stmt = nullptr;
	sqlite3_prepare_v2(db,
		"INSERT OR IGNORE INTO ammo_data (bsg_id, short_name) VALUES (?, ?);",
		-1, &stmt, nullptr);

	for (const auto& ammo : json["data"]["ammo"])
	{
		std::string id = ammo["item"]["id"].get<std::string>();
		std::string shortName = ammo["item"]["shortName"].get<std::string>();

		sqlite3_bind_text(stmt, 1, id.c_str(), -1, SQLITE_TRANSIENT);
		sqlite3_bind_text(stmt, 2, shortName.c_str(), -1, SQLITE_TRANSIENT);
		sqlite3_step(stmt);
		sqlite3_reset(stmt);
	}

	sqlite3_finalize(stmt);
	sqlite3_exec(db, "COMMIT;", nullptr, nullptr, nullptr);
	std::println("[TarkovDevAPI] FetchAmmo: done.");
}

void TarkovDevAPI::FetchExfils(sqlite3* db)
{
	constexpr const char* query = R"(
	{
	  maps {
	    nameId
	    extracts {
	      name
	      position {
	        x
	        y
	        z
	      }
	    }
	  }
	}
	)";

	// Maps tarkov.dev nameId -> list of internal game map names
	static const std::unordered_map<std::string, std::vector<std::string>> MAP_NAME_MAPPING = {
		{ "Woods",       { "woods", "Woods" } },
		{ "Shoreline",   { "shoreline", "Shoreline" } },
		{ "RezervBase",  { "rezervbase", "RezervBase" } },
		{ "Labyrinth",   { "Labyrinth" } },
		{ "laboratory",  { "laboratory" } },
		{ "Interchange", { "interchange", "Interchange" } },
		{ "factory4_day",   { "factory4_day" } },
		{ "factory4_night", { "factory4_night" } },
		{ "bigmap",      { "bigmap" } },
		{ "Lighthouse",  { "lighthouse", "Lighthouse" } },
		{ "TarkovStreets", { "tarkovstreets", "TarkovStreets" } },
		{ "Sandbox",       { "Sandbox" } },
		{ "Sandbox_high",  { "Sandbox_high" } },
		{ "Terminal",      { "Terminal" } },
		{ "Sandbox_start", { "Sandbox_start" } },
	};

	CGraphQLRequest req(API_URL, query);
	if (req.GetResponse().empty())
	{
		std::println("[TarkovDevAPI] FetchExfils: empty response, skipping.");
		return;
	}

	auto json = nlohmann::json::parse(req.GetResponse(), nullptr, false);
	if (json.is_discarded() || !json.contains("data") || !json["data"].contains("maps"))
	{
		std::println("[TarkovDevAPI] FetchExfils: failed to parse response.");
		return;
	}

	sqlite3_exec(db, "BEGIN TRANSACTION;", nullptr, nullptr, nullptr);

	sqlite3_stmt* stmt = nullptr;
	sqlite3_prepare_v2(db,
		"INSERT OR REPLACE INTO exfil_data (map_internal_name, exfil_display_name, pos_x, pos_y, pos_z) VALUES (?, ?, ?, ?, ?);",
		-1, &stmt, nullptr);

	for (const auto& map : json["data"]["maps"])
	{
		std::string nameId = map["nameId"].get<std::string>();

		auto it = MAP_NAME_MAPPING.find(nameId);
		if (it == MAP_NAME_MAPPING.end())
			continue;

		for (const auto& extract : map["extracts"])
		{
			std::string extractName = extract["name"].get<std::string>();
			double posX = extract["position"]["x"].get<double>();
			double posY = extract["position"]["y"].get<double>();
			double posZ = extract["position"]["z"].get<double>();

			for (const auto& internalName : it->second)
			{
				sqlite3_bind_text(stmt, 1, internalName.c_str(), -1, SQLITE_TRANSIENT);
				sqlite3_bind_text(stmt, 2, extractName.c_str(), -1, SQLITE_TRANSIENT);
				sqlite3_bind_double(stmt, 3, posX);
				sqlite3_bind_double(stmt, 4, posY);
				sqlite3_bind_double(stmt, 5, posZ);
				sqlite3_step(stmt);
				sqlite3_reset(stmt);
			}
		}
	}

	sqlite3_finalize(stmt);
	sqlite3_exec(db, "COMMIT;", nullptr, nullptr, nullptr);
	std::println("[TarkovDevAPI] FetchExfils: done.");
}
