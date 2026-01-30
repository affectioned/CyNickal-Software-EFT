#pragma once

#include "sqlite3.h"
#include "Game/Classes/CUnityTransform/CUnityTransform.h"

class Database
{
public:
	static void Initialize();
	[[nodiscard]] static sqlite3* GetTarkovDB();
	static bool IsDBOnFile();

private:
	static void CreateLocalDB();
	static inline sqlite3* m_TarkovDB{ nullptr };
};


class TarkovItemData
{
public:
	static int GetPriceOfItem(const std::string& item_id)
	{
		auto db = Database::GetTarkovDB();

		const char* QueryStatement = "SELECT trader_price FROM item_data WHERE bsg_id = ?;";
		sqlite3_stmt* stmt{ nullptr };
		sqlite3_prepare_v2(db, QueryStatement, -1, &stmt, nullptr);
		sqlite3_bind_text(stmt, 1, item_id.c_str(), -1, SQLITE_STATIC);
		int price_amount{ -1 };

		if (sqlite3_step(stmt) == SQLITE_ROW)
			price_amount = sqlite3_column_int(stmt, 0);

		sqlite3_finalize(stmt);
		return price_amount;
	}
	static std::string GetShortNameOfItem(const std::string& item_id)
	{
		auto db = Database::GetTarkovDB();

		const char* QueryStatement = "SELECT short_name FROM item_data WHERE bsg_id = ?;";
		sqlite3_stmt* stmt{ nullptr };
		sqlite3_prepare_v2(db, QueryStatement, -1, &stmt, nullptr);
		sqlite3_bind_text(stmt, 1, item_id.c_str(), -1, SQLITE_STATIC);
		std::string short_name;
		if (sqlite3_step(stmt) == SQLITE_ROW)
		{
			const unsigned char* text = sqlite3_column_text(stmt, 0);
			short_name = std::string(reinterpret_cast<const char*>(text));
		}
		sqlite3_finalize(stmt);
		return short_name;
	}
};

class TarkovContainerData
{
public:
	static std::string GetNameOfContainer(const std::string& container_id)
	{
		auto db = Database::GetTarkovDB();
		const char* QueryStatement = "SELECT short_name FROM container_data WHERE bsg_id = ?;";
		sqlite3_stmt* stmt{ nullptr };
		sqlite3_prepare_v2(db, QueryStatement, -1, &stmt, nullptr);
		sqlite3_bind_text(stmt, 1, container_id.c_str(), -1, SQLITE_STATIC);
		std::string name;
		if (sqlite3_step(stmt) == SQLITE_ROW)
		{
			const unsigned char* text = sqlite3_column_text(stmt, 0);
			name = std::string(reinterpret_cast<const char*>(text));
		}
		sqlite3_finalize(stmt);
		return name;
	}
};

class TarkovAmmoData
{
public:
	static std::string GetNameOfAmmo(const std::string& ammo_id)
	{
		auto db = Database::GetTarkovDB();
		const char* QueryStatement = "SELECT short_name FROM ammo_data WHERE bsg_id = ?;";
		sqlite3_stmt* stmt{ nullptr };
		sqlite3_prepare_v2(db, QueryStatement, -1, &stmt, nullptr);
		sqlite3_bind_text(stmt, 1, ammo_id.c_str(), -1, SQLITE_STATIC);
		std::string name;
		if (sqlite3_step(stmt) == SQLITE_ROW)
		{
			const unsigned char* text = sqlite3_column_text(stmt, 0);
			name = std::string(reinterpret_cast<const char*>(text));
		}
		sqlite3_finalize(stmt);
		return name;
	}
};

class TarkovExfilData
{
public:
	static std::string GetDisplayNameByPosition(const std::string& map_name, const Vector3& exfil_position)
	{
		constexpr float tolerance = 5.0f;

		auto db = Database::GetTarkovDB();

		const char* QueryStatement = R"(
			SELECT exfil_display_name,
				   ((pos_x - ?) * (pos_x - ?) + 
					(pos_y - ?) * (pos_y - ?) + 
					(pos_z - ?) * (pos_z - ?)) as distance_sq
			FROM exfil_data 
			WHERE LOWER(map_internal_name) = LOWER(?)
			ORDER BY distance_sq
			LIMIT 1
		)";

		sqlite3_stmt* stmt{ nullptr };

		if (sqlite3_prepare_v2(db, QueryStatement, -1, &stmt, nullptr) != SQLITE_OK)
			return "Unknown";

		sqlite3_bind_double(stmt, 1, exfil_position.x);
		sqlite3_bind_double(stmt, 2, exfil_position.x);
		sqlite3_bind_double(stmt, 3, exfil_position.y);
		sqlite3_bind_double(stmt, 4, exfil_position.y);
		sqlite3_bind_double(stmt, 5, exfil_position.z);
		sqlite3_bind_double(stmt, 6, exfil_position.z);
		sqlite3_bind_text(stmt, 7, map_name.c_str(), -1, SQLITE_STATIC);

		std::string display_name = "Unknown";

		if (sqlite3_step(stmt) == SQLITE_ROW)
		{
			double distance = std::sqrt(sqlite3_column_double(stmt, 1));

			if (distance < tolerance)
			{
				const unsigned char* text = sqlite3_column_text(stmt, 0);
				display_name = std::string(reinterpret_cast<const char*>(text));
			}
		}

		sqlite3_finalize(stmt);
		return display_name;
	}

	static std::string GetFaction(const std::string& map_name, const std::string& internal_name)
	{
		auto db = Database::GetTarkovDB();
		const char* QueryStatement = "SELECT faction FROM exfil_data WHERE LOWER(map_internal_name) = LOWER(?) AND LOWER(exfil_internal_name) = LOWER(?);";
		sqlite3_stmt* stmt{ nullptr };
		sqlite3_prepare_v2(db, QueryStatement, -1, &stmt, nullptr);
		sqlite3_bind_text(stmt, 1, map_name.c_str(), -1, SQLITE_STATIC);
		sqlite3_bind_text(stmt, 2, internal_name.c_str(), -1, SQLITE_STATIC);

		std::string faction;
		if (sqlite3_step(stmt) == SQLITE_ROW)
		{
			const unsigned char* text = sqlite3_column_text(stmt, 0);
			faction = std::string(reinterpret_cast<const char*>(text));
		}

		sqlite3_finalize(stmt);
		return faction;
	}
};