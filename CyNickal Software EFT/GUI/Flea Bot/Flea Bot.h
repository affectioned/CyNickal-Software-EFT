#pragma once
#include "json.hpp"

enum class EResponseType : uint8_t
{
	Offer = 0,
	INVALID = std::numeric_limits<uint8_t>::max()
};
enum class EActionType : uint8_t
{
	Refresh = 0,
	BuyTopItem = 1,
	INVALID = std::numeric_limits<uint8_t>::max()
};
const std::string RoubleBSGID{ "5449016a4bdc2d6f028b456f" };
struct CMousePos
{
	int32_t x{ 0 };
	int32_t y{ 0 };

	const CMousePos operator-(CMousePos& rhs) const
	{
		return CMousePos{ x - rhs.x, y - rhs.y };
	}
	const CMousePos operator+(CMousePos& rhs) const
	{
		return CMousePos{ x + rhs.x, y + rhs.y };
	}
};

class FleaBot
{
public:
	static void Render();
	static void OnNewResponse();
	static void InputThread();

public:
	static inline std::mutex LastOfferMut{};
	static inline nlohmann::json LatestOfferJson{};

private:
	static EResponseType IdentifyResponse(const nlohmann::json& ResponseJson);
	static void PrintOffers(const nlohmann::json& ResponseJson);
	static bool DoesOfferMeetCriteria(const nlohmann::json& OfferJson);
	static void ClickInFiveSeconds();
	static void BuyFirstItemStack(CMousePos StartingPos);

public:
	static inline bool bMasterToggle{ false };
	static inline bool bLimitBuy{ false };
	static inline bool bCycleBuildingMaterials{ true };
	static inline bool bRequestedBuy{ false };
	static inline std::unique_ptr<std::thread> pInputThread{ nullptr };
	static inline std::atomic<bool>bInputThreadDone{ false };

private:
	static void LimitBuyOneLogic(CMousePos StartingPos);
	static void CycleBuildingMaterials(CMousePos StartingPos);

private:
	static inline std::atomic<bool> bHasNewOfferData{ false };
	static nlohmann::json AwaitNewOfferData(std::chrono::milliseconds Timeout);

private:
	static uint32_t GetMaxPrice(const std::string& ItemID)
	{
		auto It = m_ItemPriceLimits.find(ItemID);
		if (It != m_ItemPriceLimits.end())
			return It->second;

		return 0;
	}
	static inline std::unordered_map<std::string, uint32_t> m_ItemPriceLimits{
		{"5d1b32c186f774252167a530",22000},	// Analog thermometer
		{"57347c5b245977448d35f6e1",14000},	// Bolts
		{"59e35cbb86f7741778269d83",35000},	// Corrugated hose
		{"57347c1124597737fb1379e3",9000},	// Duct Tape
		{"5734795124597738002c6176",9000},	// Insulating tape
		{"5e2af29386f7746d4159f077",15000},	// Kektape
		{"61bf7b6302b3924be92fa8c3",27000},	// Metal spare parts
		{"619cbf476b8a1b37a54eebf8",33000},	// Military tube
		{"590c31c586f774245e3141b2",9000},	// Pack of nails
		{"59e35ef086f7741777737012",6000},	// Pack of screws
		{"59e366c186f7741778269d85",9500},	// Plexiglass
		{"5d1b327086f7742525194449",28000},	// Pressure gauge
		{"57347c77245977448d35f6e2",6500},	// Screw nuts
		{"590c35a486f774273531c822",25000},	// Shustrillo Foam
		{"5d1b39a386f774252339976f",9000},	// Silicone tube
		{"5e2af22086f7746d3f3c33fa",11000}, // Cold welding
		{"590c346786f77423e50ed342",25000}, // Xenomorph Foam
	};
};