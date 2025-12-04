#include "pch.h"
#include <fstream>
#include "EFT.h"
#include "Classes/CGameObjectManager.h"
#include "Classes/CLinkedListEntry.h"
#include "Classes/CObject.h"
#include "Classes/CPlayer.h"

#include "Player List/Player List.h"

#include "Offsets/Offsets.h"

std::vector<uintptr_t> LocalPlayerAddresses{};
void PopulatePlayers(DMA_Connection* Conn, uintptr_t LocalGameWorld)
{
	uintptr_t PlayerListAddress = LocalGameWorld + Offsets::CLocalGameWorld::pRegisteredPlayers;

	auto& Proc = EFT::GetProcess();

	uintptr_t RegisteredPlayersAddress = Proc.ReadMem<uintptr_t>(Conn, PlayerListAddress);

	uintptr_t PlayerDataArrayAddress = Proc.ReadMem<uintptr_t>(Conn, RegisteredPlayersAddress + Offsets::CRegisteredPlayers::pPlayerArray);
	uintptr_t NumPlayersAddress = RegisteredPlayersAddress + Offsets::CRegisteredPlayers::NumPlayers;
	uintptr_t MaxPlayersAddress = RegisteredPlayersAddress + Offsets::CRegisteredPlayers::MaxPlayers;

	uint32_t NumPlayers = Proc.ReadMem<uint32_t>(Conn, NumPlayersAddress);
	uint32_t MaxPlayers = Proc.ReadMem<uint32_t>(Conn, MaxPlayersAddress);

	std::println("[Player List] Player Data: 0x{:X}", PlayerDataArrayAddress);
	std::println("[Player List] NumPlayers: {}", NumPlayers);
	std::println("[Player List] MaxPlayers: {}", MaxPlayers);

	if (NumPlayers > 128)
		throw std::runtime_error("Unreasonable number of players detected.");

	LocalPlayerAddresses.resize(NumPlayers);
	auto vmsh = VMMDLL_Scatter_Initialize(Conn->GetHandle(), Proc.GetPID(), VMMDLL_FLAG_NOCACHE);
	VMMDLL_Scatter_PrepareEx(vmsh, PlayerDataArrayAddress + 0x20, sizeof(uintptr_t) * NumPlayers, reinterpret_cast<BYTE*>(LocalPlayerAddresses.data()), nullptr);
	VMMDLL_Scatter_Execute(vmsh);
	VMMDLL_Scatter_CloseHandle(vmsh);

	PlayerList::m_PlayerAddr = LocalPlayerAddresses;
}

bool EFT::Initialize(DMA_Connection* Conn)
{
	std::println("Initializing EFT module...");

	Proc.GetProcessInfo(Conn);

	uintptr_t pGOMAddress = Proc.GetUnityAddress() + Offsets::pGOM;
	GameObjectManagerAddress = Proc.ReadMem<uintptr_t>(Conn, pGOMAddress);
	std::println("GameObjectManager Address: 0x{:X}", GameObjectManagerAddress);

	LastActiveNode = Proc.ReadMem<uintptr_t>(Conn, GameObjectManagerAddress + Offsets::CGameObjectManager::pLastActiveNode);
	std::println("LastActiveNode Address: 0x{:X}", LastActiveNode);

	ActiveNodes = Proc.ReadMem<uintptr_t>(Conn, GameObjectManagerAddress + Offsets::CGameObjectManager::pActiveNodes);
	std::println("ActiveNodes Address: 0x{:X}", ActiveNodes);

	GetObjectAddresses(Conn);

	PopulateObjectInfoListFromAddresses(Conn);

	//DumpAllObjectsToFile("ObjectDump.txt");

	try
	{
		uintptr_t LocalGameWorld = GetLocalGameWorldAddr(Conn);
		PopulatePlayers(Conn, LocalGameWorld);

		auto MainPlayer = Proc.ReadMem<uintptr_t>(Conn, LocalGameWorld + offsetof(CLocalGameWorld, pMainPlayer));
		std::println("[EFT] MainPlayer Address: 0x{:X}", MainPlayer);

		PlayerList::FullUpdate(Conn);

		for (int i = 0; i < 100; i++)
		{
			PlayerList::QuickUpdate(Conn);
			PlayerList::PrintPlayers();
			std::this_thread::sleep_for(std::chrono::seconds(1));
		}
	}
	catch (const std::exception& e)
	{
		std::println("[EFT] Initialize; Exception: {}", e.what());
		return false;
	}

	////auto PlayerMovementContext = Proc.ReadMem<uintptr_t>(Conn, MainPlayer + offsetof(CPlayer, pMovementContext));
	////std::println("[EFT] Main PlayerMovementContext Address: 0x{:X}", PlayerMovementContext);

	////auto PlayerMovementContextYaw = Proc.ReadMem<float>(Conn, PlayerMovementContext + offsetof(CMovementContext, Yaw));
	////std::println("[EFT] Main Player Yaw: {}", PlayerMovementContextYaw);

	return false;
}

const Process& EFT::GetProcess()
{
	return Proc;
}

void EFT::GetObjectAddresses(DMA_Connection* Conn, uint32_t MaxNodes)
{
	m_ObjectAddresses.clear();

	auto StartTime = std::chrono::high_resolution_clock::now();
	uint32_t NodeCount = 0;
	DWORD BytesRead = 0;
	uintptr_t CurrentActiveNode = ActiveNodes;
	uintptr_t FirstNode = ActiveNodes;

	auto vmsh = VMMDLL_Scatter_Initialize(Conn->GetHandle(), Proc.GetPID(), 0);
	while (true)
	{
		if (CurrentActiveNode == LastActiveNode && NodeCount > 5)
			break;

		if (NodeCount >= MaxNodes)
			break;

		CLinkedListEntry NodeEntry{};
		VMMDLL_Scatter_PrepareEx(vmsh, CurrentActiveNode, sizeof(CLinkedListEntry), reinterpret_cast<BYTE*>(&NodeEntry), &BytesRead);
		VMMDLL_Scatter_Execute(vmsh);
		VMMDLL_Scatter_Clear(vmsh, Proc.GetPID(), 0);

		if (BytesRead != sizeof(CLinkedListEntry))
		{
			std::println("[EFT] UpdateObjectList; Incomplete read: {0:d}/{1:d} @ {2:X}", BytesRead, sizeof(CLinkedListEntry), CurrentActiveNode);
			break;
		}

		m_ObjectAddresses.push_back(NodeEntry.pObject);

		if (NodeEntry.pNextEntry == FirstNode)
		{
			std::println("[EFT] UpdateObjectList; Reached back to first node, ending traversal.");
			break;
		}

		if (NodeEntry.pNextEntry == 0)
		{
			std::println("[EFT] UpdateObjectList; Next entry is null, ending traversal.");
			break;
		}

		CurrentActiveNode = NodeEntry.pNextEntry;
		NodeCount++;
	}

	auto EndTime = std::chrono::high_resolution_clock::now();
	auto Duration = std::chrono::duration_cast<std::chrono::milliseconds>(EndTime - StartTime).count();
	std::println("[EFT] UpdateObjectList; {} nodes in {}ms", NodeCount, Duration);
}

std::vector<uintptr_t> EFT::GetGameWorldAddresses(DMA_Connection* Conn)
{
	std::vector<uintptr_t> GameWorldAddresses{};

	for (auto& ObjInfo : m_ObjectInfo)
	{
		if (ObjInfo.m_ObjectName == "GameWorld")
		{
			std::println("[EFT] GameWorld Address: 0x{:X}", ObjInfo.m_ObjectAddress);
			GameWorldAddresses.push_back(ObjInfo.m_ObjectAddress);
		}
	}

	return GameWorldAddresses;
}

struct MapNameBuff
{
	wchar_t MapName[64]{ 0 };
};
uintptr_t EFT::GetLocalGameWorldAddr(DMA_Connection* Conn)
{
	auto GameWorldAddrs = GetGameWorldAddresses(Conn);

	wchar_t MapNameBuffer[64]{};
	uintptr_t LocalWorldAddr = 0;

	for (auto& GameWorldAddr : GameWorldAddrs)
	{
		auto Deref1 = Proc.ReadMem<uintptr_t>(Conn, GameWorldAddr + 0x58);
		auto Deref2 = Proc.ReadMem<uintptr_t>(Conn, Deref1 + 0x18);
		LocalWorldAddr = Proc.ReadMem<uintptr_t>(Conn, Deref2 + 0x30);
		auto MainPlayerAddr = Proc.ReadMem<uintptr_t>(Conn, LocalWorldAddr + Offsets::CLocalGameWorld::pMainPlayer);
		auto MapNameAddr = Proc.ReadMem<uintptr_t>(Conn, LocalWorldAddr + Offsets::CLocalGameWorld::pMapName);
		MapNameBuff buffer = Proc.ReadMem<MapNameBuff>(Conn, MapNameAddr + 0x14);
		std::wstring MapNameStr(buffer.MapName);
		std::string MapNameNarrow(MapNameStr.begin(), MapNameStr.end());

		std::println("[EFT] LocalGameWorld @ Map Name: {}", MapNameNarrow);
		std::println("[EFT] LocalGameWorld Address: 0x{:X}\n", LocalWorldAddr);
	}

	return LocalWorldAddr;

	throw std::runtime_error("Failed to find valid LocalGameWorld address.");
}

void EFT::DumpAllObjectsToFile(const std::string& FileName)
{
	std::ofstream OutFile(FileName, std::ios::out | std::ios::trunc);
	if (!OutFile.is_open())
	{
		std::println("[EFT] DumpAllObjectsToFile; Failed to open file: {}", FileName);
		return;
	}

	for (int i = 0; i < m_ObjectInfo.size(); i++)
	{
		auto& ObjInfo = m_ObjectInfo[i];
		OutFile << std::format("Entity #{0:d} @ {1:X} named `{2:s}`", i, ObjInfo.m_ObjectAddress, ObjInfo.m_ObjectName.c_str()) << std::endl;
	}

	OutFile.close();

}

std::vector<std::pair<uintptr_t, std::array<std::byte, 32>>> ObjectDataBuffers{};
void EFT::PopulateObjectInfoListFromAddresses(DMA_Connection* Conn)
{
	ObjectDataBuffers.resize(m_ObjectAddresses.size());

	auto vmsh = VMMDLL_Scatter_Initialize(Conn->GetHandle(), Proc.GetPID(), VMMDLL_FLAG_NOCACHE);

	for (int i = 0; i < m_ObjectAddresses.size(); i++)
	{
		auto& ObjAddr = m_ObjectAddresses[i];
		uintptr_t NameAddress = ObjAddr + offsetof(CObject, pName);
		VMMDLL_Scatter_PrepareEx(vmsh, NameAddress, sizeof(uintptr_t), reinterpret_cast<BYTE*>(&ObjectDataBuffers[i].first), nullptr);
	}

	VMMDLL_Scatter_Execute(vmsh);

	VMMDLL_Scatter_Clear(vmsh, Proc.GetPID(), VMMDLL_FLAG_NOCACHE);

	for (int i = 0; i < m_ObjectAddresses.size(); i++)
	{
		auto& [NameAddress, DataBuffer] = ObjectDataBuffers[i];
		VMMDLL_Scatter_PrepareEx(vmsh, NameAddress, DataBuffer.size(), reinterpret_cast<BYTE*>(DataBuffer.data()), nullptr);
	}

	VMMDLL_Scatter_Execute(vmsh);

	VMMDLL_Scatter_CloseHandle(vmsh);

	m_ObjectInfo.clear();
	for (int i = 0; i < m_ObjectAddresses.size(); i++)
	{
		auto& [NameAddress, DataBuffer] = ObjectDataBuffers[i];
		std::string Name(reinterpret_cast<char*>(DataBuffer.data()), strnlen_s(reinterpret_cast<char*>(DataBuffer.data()), DataBuffer.size()));
		CObjectInfo ObjInfo{};
		ObjInfo.m_ObjectAddress = m_ObjectAddresses[i];
		ObjInfo.m_ObjectName = Name;
		m_ObjectInfo.push_back(ObjInfo);
	}
}