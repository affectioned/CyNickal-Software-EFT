#include "pch.h"

#include "Player List.h"

#include "Game/EFT.h"

void PlayerList::FullUpdate(DMA_Connection* Conn)
{
	std::scoped_lock Lock(m_PlayerMutex);

	m_Players.clear();

	const auto PID = EFT::GetProcess().GetPID();

	for (auto& PlayerAddr : m_PlayerAddr)
		m_Players.emplace_back(CEFTPlayer(PlayerAddr));

	auto vmsh = VMMDLL_Scatter_Initialize(Conn->GetHandle(), PID, VMMDLL_FLAG_NOCACHE);

	for (auto& Player : m_Players)
		Player.PrepareRead_1(vmsh);
	VMMDLL_Scatter_Execute(vmsh);
	VMMDLL_Scatter_Clear(vmsh, PID, VMMDLL_FLAG_NOCACHE);

	for (auto& Player : m_Players)
		Player.PrepareRead_2(vmsh);
	VMMDLL_Scatter_Execute(vmsh);
	VMMDLL_Scatter_Clear(vmsh, PID, VMMDLL_FLAG_NOCACHE);

	for (auto& Player : m_Players)
		Player.PrepareRead_3(vmsh);
	VMMDLL_Scatter_Execute(vmsh);
	VMMDLL_Scatter_Clear(vmsh, PID, VMMDLL_FLAG_NOCACHE);

	for (auto& Player : m_Players)
		Player.PrepareRead_4(vmsh);
	VMMDLL_Scatter_Execute(vmsh);
	VMMDLL_Scatter_Clear(vmsh, PID, VMMDLL_FLAG_NOCACHE);

	for (auto& Player : m_Players)
		Player.PrepareRead_5(vmsh);
	VMMDLL_Scatter_Execute(vmsh);
	VMMDLL_Scatter_Clear(vmsh, PID, VMMDLL_FLAG_NOCACHE);

	for (auto& Player : m_Players)
		Player.PrepareRead_6(vmsh);
	VMMDLL_Scatter_Execute(vmsh);
	VMMDLL_Scatter_Clear(vmsh, PID, VMMDLL_FLAG_NOCACHE);

	for (auto& Player : m_Players)
		Player.PrepareRead_7(vmsh);
	VMMDLL_Scatter_Execute(vmsh);
	VMMDLL_Scatter_Clear(vmsh, PID, VMMDLL_FLAG_NOCACHE);

	for (auto& Player : m_Players)
		Player.PrepareRead_8(vmsh);
	VMMDLL_Scatter_Execute(vmsh);
	VMMDLL_Scatter_CloseHandle(vmsh);

	for (auto& Player : m_Players)
		Player.Finalize();
}

void PlayerList::QuickUpdate(DMA_Connection* Conn)
{
	std::scoped_lock Lock(m_PlayerMutex);

	auto vmsh = VMMDLL_Scatter_Initialize(Conn->GetHandle(), EFT::GetProcess().GetPID(), VMMDLL_FLAG_NOCACHE);
	for (auto& Player : m_Players)
		Player.QuickRead(vmsh);
	VMMDLL_Scatter_Execute(vmsh);
	VMMDLL_Scatter_CloseHandle(vmsh);

	for (auto& Player : m_Players)
		Player.Finalize();
}

void PlayerList::PrintPlayers()
{
	std::scoped_lock Lock(m_PlayerMutex);
	for (int i = 0; i < m_Players.size(); i++)
	{
		auto& Player = m_Players[i];
		if (Player.IsInvalid()) continue;
		std::println("Player #{0:d} @ 0x{1:X} - Base Position: X: {2:f}, Y: {3:f}, Z: {4:f}", i, Player.m_EntityAddress, Player.m_BasePosition.x, Player.m_BasePosition.y, Player.m_BasePosition.z);
	}
	std::println("");
}
