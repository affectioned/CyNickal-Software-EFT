#include "pch.h"
#include "Flea Bot.h"
#include "Game/Response Data/Response Data.h"
#include "Makcu/MyMakcu.h"

using namespace std::chrono_literals;

void FleaBot::Render()
{
	if (pInputThread && !bMasterToggle && bInputThreadDone)
	{
		std::println("[{0:%T}] [Flea Bot] Stopping input thread.", std::chrono::system_clock::now());
		pInputThread->join();
		pInputThread = nullptr;
	};

	if (!pInputThread && bMasterToggle)
	{
		bInputThreadDone = false;
		pInputThread = std::make_unique<std::thread>(&FleaBot::InputThread);
	}

	ImGui::Begin("Flea Bot");
	ImGui::Checkbox("Enable Flea Bot", &bMasterToggle);

	ImGui::Checkbox("Cycle Building Materials", &bCycleBuildingMaterials);
	ImGui::Checkbox("Limit Buying", &bLimitBuy);
	ImGui::End();
}

void FleaBot::OnNewResponse()
{
	auto& Json = ResponseData::LatestJson;

	auto ResponseType = IdentifyResponse(Json);

	if (ResponseType == EResponseType::Offer)
	{
		bHasNewOfferData = true;
		std::scoped_lock Lock(LastOfferMut);
		LatestOfferJson = Json;
	}
}

void FleaBot::InputThread()
{
	std::println("[{0:%T}] [Flea Bot] Input thread started.", std::chrono::system_clock::now());

	while (bMasterToggle)
	{
		if (bLimitBuy)
			LimitBuyOneLogic({ 1840,120 });

		if (bCycleBuildingMaterials)
			CycleBuildingMaterials({ 1840,120 });

		std::this_thread::sleep_for(std::chrono::milliseconds(25));
	}

	std::println("[{0:%T}] [Flea Bot] Input thread joining", std::chrono::system_clock::now());
	bInputThreadDone = true;
}

EResponseType FleaBot::IdentifyResponse(const nlohmann::json& ResponseJson)
{
	EResponseType Return = EResponseType::INVALID;

	if (ResponseJson.contains("data") == false) return Return;

	if (ResponseJson["data"].contains("offers"))
		Return = EResponseType::Offer;

	return Return;
}

void FleaBot::PrintOffers(const nlohmann::json& ResponseJson)
{
	const auto& Offers = ResponseJson["data"]["offers"];

	for (auto& Offer : Offers)
	{
		auto Username = Offer["user"]["nickname"].get<std::string>();
		auto ItemID = Offer["items"][0]["_tpl"].get<std::string>();
		auto Quantity = Offer["quantity"].get<int>();
		auto Price = Offer["requirements"][0]["count"].get<int>();
		std::println("   {} offering {} {} for {} each", Username.c_str(), ItemID, Quantity, Price);
	}
}

bool FleaBot::DoesOfferMeetCriteria(const nlohmann::json& OfferJson)
{
	auto Price = OfferJson["requirements"][0]["count"].get<int>();
	auto ItemId = OfferJson["items"][0]["_tpl"].get<std::string>();
	auto CurrencyType = OfferJson["requirements"][0]["_tpl"].get<std::string>();
	auto Quantity = OfferJson["quantity"].get<int>();

	if (Quantity < 1)
		return false;

	if (CurrencyType != RoubleBSGID)
		return false;

	uint32_t MaxPriceForItem = GetMaxPrice(ItemId);

	if (Price > MaxPriceForItem)
		return false;

	std::println("[{0:%T}] [Flea Bot] Buying item {1:s} x {2:d} for {3:d}", std::chrono::system_clock::now(), ItemId, Quantity, Price);

	return true;
}

void FleaBot::ClickInFiveSeconds()
{
	std::this_thread::sleep_for(std::chrono::seconds(5));
	MyMakcu::m_Device.click(makcu::MouseButton::LEFT);
}

const auto ShortDelay = 50ms;
const auto MediumDelay = std::chrono::milliseconds(500);
const auto LongDelay = std::chrono::seconds(2);
void ClickThenWait(std::chrono::milliseconds delay)
{
	MyMakcu::m_Device.mouseDown(makcu::MouseButton::LEFT);
	std::this_thread::sleep_for(10ms);
	MyMakcu::m_Device.mouseUp(makcu::MouseButton::LEFT);
	std::this_thread::sleep_for(delay);
}
CMousePos MoveThenWait(CMousePos PreviousPos, CMousePos Delta, std::chrono::milliseconds delay)
{
	auto FinalPos = PreviousPos + Delta;
	MyMakcu::m_Device.mouseMove(Delta.x, Delta.y);
	std::this_thread::sleep_for(delay);
	return FinalPos;
}
CMousePos MoveThenClick(CMousePos PreviousPos, CMousePos Delta, std::chrono::milliseconds delay)
{
	auto FinalPos = PreviousPos + Delta;

	MoveThenWait(PreviousPos, Delta, delay);

	ClickThenWait(15ms);

	return FinalPos;
}

void FleaBot::BuyFirstItemStack(CMousePos StartingPos)
{
	CMousePos DesiredPos{ 1775, 180 };
	auto Delta = DesiredPos - StartingPos;

	auto CurPos = MoveThenClick(StartingPos, Delta, ShortDelay);
	CurPos = MoveThenClick(CurPos, { -625,310 }, ShortDelay);
	CurPos = MoveThenClick(CurPos, { -285,115 }, ShortDelay);

	/* Wait for order to process */
	std::this_thread::sleep_for(LongDelay);
	CurPos = MoveThenClick(CurPos, { 95, -25 }, ShortDelay);

	Delta = StartingPos - CurPos;
	MyMakcu::m_Device.mouseMove(Delta.x, Delta.y);
}

void FleaBot::CycleBuildingMaterials(CMousePos StartingPos)
{
	const auto ConstructionListStart = CMousePos{ 559,240 };
	const auto Delta = ConstructionListStart - StartingPos;

	auto CurPos = MoveThenWait(StartingPos, Delta, 300ms);
	bool bFailed{ false };

	constexpr size_t ItemRows = 17;
	uint32_t FailedAttempts{ 0 };
	for (int i = 0; i < ItemRows && !bFailed && bMasterToggle; i++)
	{
		bHasNewOfferData = false;
		CurPos = MoveThenClick(CurPos, { 0,25 }, 300ms);
		auto OfferJson = AwaitNewOfferData(1500ms);

		if (OfferJson.is_null())
		{
			FailedAttempts++;
			std::println("[{0:%T}] [Flea Bot] No new data received for item index {1:d}; failed attempts: {2:d}", std::chrono::system_clock::now(), i + 1, FailedAttempts);

			if (FailedAttempts >= 5)
				bFailed = true;

			continue;
		}

		auto& FirstOffer = OfferJson["data"]["offers"][0];
		if (DoesOfferMeetCriteria(FirstOffer))
		{
			/* Waiting for response to populate UI */
			std::this_thread::sleep_for(20ms);
			BuyFirstItemStack(CurPos);
		}
	}

	if (bFailed)
	{
		std::println("[{0:%T}] [Flea Bot] Too many failed attempts; stopping flea bot.", std::chrono::system_clock::now());
		bMasterToggle = false;
		return;
	}

	if (!bMasterToggle)
		return;

	/* click on the category following the last item */
	CurPos = MoveThenClick(CurPos, { 0,25 }, 500ms);

	/* click refresh and wait for response */
	MoveThenClick(CurPos, StartingPos - CurPos, ShortDelay);
	bHasNewOfferData = false;
	auto Response = AwaitNewOfferData(5000ms);
	if (Response.is_null())
	{
		std::println("[{0:%T}] [Flea Bot] No new data received after cycling building materials.", std::chrono::system_clock::now());
		bMasterToggle = false;
	}
}

nlohmann::json FleaBot::AwaitNewOfferData(std::chrono::milliseconds Timeout)
{
	const auto StartTime = std::chrono::steady_clock::now();
	bool bFailed{ false };
	auto Return = nlohmann::json();

	while (bHasNewOfferData == false && bFailed == false)
	{
		std::chrono::milliseconds Elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - StartTime);

		if (Elapsed >= Timeout)
		{
			bFailed = true;
			std::println("[{0:%T}] [Flea Bot] AwaitNewOfferData timed out after {1:d} ms.", std::chrono::system_clock::now(), Elapsed.count());
		}
	}

	if (bFailed == false)
	{
		std::scoped_lock Lock(LastOfferMut);
		Return = LatestOfferJson;
	}

	return Return;
}

void FleaBot::LimitBuyOneLogic(CMousePos StartingPos)
{
	std::scoped_lock Lock(LastOfferMut);

	if (LatestOfferJson.contains("data") == false || LatestOfferJson["data"].contains("offers") == false || LatestOfferJson["data"]["offers"].empty())
		return;

	auto& BestOffer = LatestOfferJson["data"]["offers"][0];
	auto OfferId = BestOffer["_id"].get<std::string>();
	static std::string LastOfferId{ "" };
	if (OfferId != LastOfferId && DoesOfferMeetCriteria(LatestOfferJson["data"]["offers"][0]))
	{
		BuyFirstItemStack(StartingPos);
		LastOfferId = OfferId;
	}

	static std::chrono::time_point<std::chrono::steady_clock> LastRefreshTime = std::chrono::steady_clock::now();
	auto CurrentTime = std::chrono::steady_clock::now();
	if (std::chrono::duration_cast<std::chrono::seconds>(CurrentTime - LastRefreshTime).count() > 4)
	{
		MyMakcu::m_Device.click(makcu::MouseButton::LEFT);
		LastRefreshTime = CurrentTime;
	}
}