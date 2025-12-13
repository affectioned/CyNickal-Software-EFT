#include "pch.h"
#include "EFT.h"
#include "Game/GOM/GOM.h"
#include "Game/Camera/Camera.h"
#include "Game/Loot List/Loot List.h"
#include "Game/Exfil List/Exfil List.h"

bool EFT::Initialize(DMA_Connection* Conn)
{
	std::println("Initializing EFT module...");

	Proc.GetProcessInfo(Conn);

	try
	{
		GOM::Initialize(Conn);
		m_CachedLocalWorldAddress = GOM::FindGameWorldAddressFromCache(Conn);

		Camera::Initialize(Conn);
		LootList::CompleteUpdate(Conn);
		ExfilList::Initialize(Conn);
	}
	catch (const std::exception& e)
	{
		std::println("EFT Initialization failed: {}", e.what());
		system("pause");
		return false;
	}

	return true;
}

const Process& EFT::GetProcess()
{
	return Proc;
}

uintptr_t EFT::GetCachedWorldAddress()
{
	return m_CachedLocalWorldAddress;
}