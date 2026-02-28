#include "pch.h"
#include "DMA Thread.h"
#include "Input Manager.h"
#include "Game/EFT.h"
#include "Game/Response Data/Response Data.h"

#include "Game/GOM/GOM.h"
#include "Game/Camera List/Camera List.h"
#include "GUI/Aimbot/Aimbot.h"
#include "GUI/Keybinds/Keybinds.h"
#include "Game/Offsets/Offsets.h"

extern std::atomic<bool> bRunning;
static std::atomic<bool> bInRaid = false;
static std::atomic<bool> s_bWorldUpdateRunning{ false };

void DMA_Thread_Main()
{
	std::println("[DMA Thread] DMA Thread started.");

	DMA_Connection* Conn = DMA_Connection::GetInstance();

	c_keys::InitKeyboard(Conn);

	if (!EFT::Initialize(Conn))
	{
		std::println("[DMA Thread] EFT Initialization failed, requesting exit.");
		bRunning = false;
		return;
	}

	CTimer LightRefresh(std::chrono::seconds(5), [&Conn]() { Conn->LightRefresh(); });

	// RaidCheck runs CreateWorldIfNeeded on a background thread so the slow GOM scan
	// (up to 20,000 serial DMA round-trips, ~3-5 s) never blocks the DMA loop.
	// s_bWorldUpdateRunning prevents concurrent world-update threads.
	CTimer RaidCheck(std::chrono::seconds(10), [&Conn]() {
		if (s_bWorldUpdateRunning.exchange(true)) return;
		std::thread([&Conn]() {
			EFT::CreateWorldIfNeeded(Conn);
			s_bWorldUpdateRunning = false;
		}).detach();
	});

	CTimer ResponseData(std::chrono::milliseconds(25), [&Conn]() {
		ResponseData::OnDMAFrame(Conn);
		});

	// Shared lock lets camera/player timers run concurrently with each other and with
	// CreateWorldIfNeeded's IsValidRaid check. Only the brief pointer swap in
	// CreateWorldIfNeeded takes an exclusive lock, so these never block for more than
	// a few microseconds.
	CTimer Player_Quick(std::chrono::milliseconds(25), [&Conn]() {
		std::shared_lock Lock(EFT::m_GameWorldMutex);
		if (EFT::pGameWorld) EFT::QuickUpdatePlayers(Conn);
		});
	CTimer Player_Allocations(std::chrono::seconds(5), [&Conn]() {
		std::shared_lock Lock(EFT::m_GameWorldMutex);
		if (EFT::pGameWorld) EFT::HandlePlayerAllocations(Conn);
		});
	CTimer Camera_UpdateViewMatrix(std::chrono::milliseconds(2), [&Conn]() {
		std::shared_lock Lock(EFT::m_GameWorldMutex);
		if (EFT::pGameWorld) CameraList::QuickUpdateNecessaryCameras(Conn);
		});
	CTimer Keybinds(std::chrono::milliseconds(50), [&Conn]() { Keybinds::OnDMAFrame(Conn); });

	while (bRunning)
	{
		auto TimeNow = std::chrono::high_resolution_clock::now();
		LightRefresh.Tick(TimeNow);
		RaidCheck.Tick(TimeNow);
		ResponseData.Tick(TimeNow);
		Player_Quick.Tick(TimeNow);
		Player_Allocations.Tick(TimeNow);
		Camera_UpdateViewMatrix.Tick(TimeNow);
		Keybinds.Tick(TimeNow);
	}

	Conn->EndConnection();
}