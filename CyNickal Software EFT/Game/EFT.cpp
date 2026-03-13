#include "pch.h"
#include "EFT.h"
#include "Game/GOM/GOM.h"
#include "Game/Camera List/Camera List.h"
#include "Offsets/Offsets.h"

bool EFT::Initialize(DMA_Connection* Conn)
{
	std::println("Initializing EFT module...");

	Proc.GetProcessInfo(Conn);

	Offsets::ResolveOffsets(Conn);

	CreateWorldIfNeeded(Conn);

	CameraList::Initialize(Conn);

	return true;
}

const Process& EFT::GetProcess()
{
	return Proc;
}

void EFT::CreateWorldIfNeeded(DMA_Connection* Conn)
{
	try 
	{
		if (pGameWorld && pGameWorld->IsValidRaid(Conn)) {
			return;
		}

		std::println("[EFT] Not in raid or invalid raid detected.");

		auto LatestWorldAddr = GOM::GetLatestWorldAddr(Conn);

		{
			std::scoped_lock Lock(m_GameWorldMutex);

			pGameWorld.reset();

			if (LatestWorldAddr) {
				pGameWorld = std::make_unique<CLocalGameWorld>(LatestWorldAddr);
			}
		}

		CameraList::Initialize(Conn);
	}
	catch (const std::exception& e)
	{
		std::println("Exception in EFT::CreateWorldIfNeeded: {}", e.what());
		return;
	}
}

uintptr_t EFT::GetMainPlayerAddress()
{
	if (pGameWorld)
		return pGameWorld->GetMainPlayerAddress();

	return uintptr_t();
}

void EFT::QuickUpdatePlayers(DMA_Connection* Conn)
{
	try
	{
		if (pGameWorld)
			pGameWorld->QuickUpdatePlayers(Conn);
	}
	catch (const std::exception& e)
	{
		std::println("Exception in EFT::QuickUpdatePlayers: {}", e.what());
		return;
	}
}

void EFT::HandlePlayerAllocations(DMA_Connection* Conn)
{
	try
	{
		if (pGameWorld)
			pGameWorld->HandlePlayerAllocations(Conn);
	}
	catch (const std::exception& e)
	{
		std::println("Exception in EFT::HandlePlayerAllocations: {}", e.what());
		return;
	}
}

CRegisteredPlayers& EFT::GetRegisteredPlayers()
{
	if (!pGameWorld)
		throw std::runtime_error("EFT::pGameWorld is null");

	if (!pGameWorld->m_pRegisteredPlayers)
		throw std::runtime_error("EFT::pGameWorld->m_pRegisteredPlayers is null");

	return *(pGameWorld->m_pRegisteredPlayers);
}

CLootList& EFT::GetLootList()
{
	if (!pGameWorld)
		throw std::runtime_error("EFT::pGameWorld is null");

	if (!pGameWorld->m_pLootList)
		throw std::runtime_error("EFT::pGameWorld->m_pLootList is null");

	return *(pGameWorld->m_pLootList);
}

CExfilController& EFT::GetExfilController()
{
	if (!pGameWorld)
		throw std::runtime_error("EFT::pGameWorld is null");

	if (!pGameWorld->m_pExfilController)
		throw std::runtime_error("EFT::pGameWorld->m_pRegisteredExfils is null");

	return *(pGameWorld->m_pExfilController);
}
