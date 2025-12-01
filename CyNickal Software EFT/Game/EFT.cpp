#include "pch.h"
#include "EFT.h"
#include "Classes/CGameObjectManager.h"
#include "Classes/CLinkedListEntry.h"
#include "Classes/CObject.h"

bool EFT::Initialize(DMA_Connection* Conn)
{
	std::println("Initializing EFT module...");

	Proc.GetProcessInfo(Conn);

	uintptr_t pGOMAddress = Proc.GetUnityAddress() + 0x1A208D8;
	GameObjectManagerAddress = Proc.ReadMem<uintptr_t>(Conn, pGOMAddress);
	std::println("GameObjectManager Address: 0x{:X}", GameObjectManagerAddress);

	LastActiveNode = Proc.ReadMem<uintptr_t>(Conn, GameObjectManagerAddress + offsetof(CGameObjectManager, pLastActiveNode));
	std::println("LastActiveNode Address: 0x{:X}", LastActiveNode);

	ActiveNodes = Proc.ReadMem<uintptr_t>(Conn, GameObjectManagerAddress + offsetof(CGameObjectManager, pActiveNodes));
	std::println("ActiveNodes Address: 0x{:X}", ActiveNodes);

	UpdateObjectList(Conn, 10000);

	uintptr_t GameWorld = GetGameWorldAddr(Conn);
	std::println("GameWorld Address: 0x{:X}", GameWorld);

	uintptr_t GameWorldComponent = Proc.ReadMem<uintptr_t>(Conn, GameWorld + 0x58);
	uintptr_t UnknownPtr = Proc.ReadMem<uintptr_t>(Conn, GameWorldComponent + 0x18);
	uintptr_t Component = Proc.ReadMem<uintptr_t>(Conn, UnknownPtr + 0x30);
	std::println("Game World Chain Final: 0x{:X}", Component);

	return false;
}

const Process& EFT::GetProcess()
{
	return Proc;
}

void EFT::UpdateObjectList(DMA_Connection* Conn, uint32_t MaxNodes)
{
	ObjectList.clear();

	auto StartTime = std::chrono::high_resolution_clock::now();
	uint32_t NodeCount = 0;
	DWORD BytesRead = 0;
	uintptr_t CurrentActiveNode = ActiveNodes;

	auto vmsh = VMMDLL_Scatter_Initialize(Conn->GetHandle(), Proc.GetPID(), VMMDLL_FLAG_NOCACHE);
	while (CurrentActiveNode != LastActiveNode && CurrentActiveNode && NodeCount <= MaxNodes)
	{
		CLinkedListEntry NodeEntry{};
		VMMDLL_Scatter_PrepareEx(vmsh, CurrentActiveNode, sizeof(CLinkedListEntry), reinterpret_cast<BYTE*>(&NodeEntry), &BytesRead);
		VMMDLL_Scatter_Execute(vmsh);
		VMMDLL_Scatter_Clear(vmsh, Proc.GetPID(), VMMDLL_FLAG_NOCACHE);

		if (BytesRead != sizeof(CLinkedListEntry))
		{
			std::println("[EFT] UpdateObjectList; Incomplete read: {0:d}/{1:d} @ {2:X}", BytesRead, sizeof(CLinkedListEntry), CurrentActiveNode);
			break;
		}

		std::println("[EFT] UpdateObjectList; Found Object at 0x{:X}", NodeEntry.pObject);
		ObjectList.push_back(NodeEntry.pObject);
		CurrentActiveNode = NodeEntry.pNextObject;
		NodeCount++;
	}

	auto EndTime = std::chrono::high_resolution_clock::now();
	auto Duration = std::chrono::duration_cast<std::chrono::milliseconds>(EndTime - StartTime).count();
	std::println("[EFT] UpdateObjectList; {} nodes in {}ms", NodeCount, Duration);
}

struct CNameBuff { char Name[64]{ 0 }; };
std::vector<std::pair<uintptr_t, DWORD>> ObjectNameAddresses{};
std::vector< CNameBuff > ObjectNameBuffers{};
uintptr_t EFT::GetGameWorldAddr(DMA_Connection* Conn)
{
	ObjectNameAddresses.resize(ObjectList.size());
	ObjectNameBuffers.resize(ObjectList.size());

	auto vmsh = VMMDLL_Scatter_Initialize(Conn->GetHandle(), Proc.GetPID(), VMMDLL_FLAG_NOCACHE);

	for (int i = 0; i < ObjectList.size(); i++)
	{
		auto& ObjAddr = ObjectList[i];
		uintptr_t NameAddress = ObjAddr + offsetof(CObject, pName);
		VMMDLL_Scatter_PrepareEx(vmsh, NameAddress, sizeof(uintptr_t), reinterpret_cast<BYTE*>(&ObjectNameAddresses[i].first), &ObjectNameAddresses[i].second);
	}

	VMMDLL_Scatter_Execute(vmsh);

	VMMDLL_Scatter_Clear(vmsh, Proc.GetPID(), VMMDLL_FLAG_NOCACHE);

	for (int i = 0; i < ObjectList.size(); i++)
	{
		auto& [NameAddress, BytesRead] = ObjectNameAddresses[i];

		if (BytesRead != sizeof(uintptr_t))
			continue;

		VMMDLL_Scatter_PrepareEx(vmsh, NameAddress, sizeof(CNameBuff), reinterpret_cast<BYTE*>(&ObjectNameBuffers[i]), &BytesRead);
	}

	VMMDLL_Scatter_Execute(vmsh);
	VMMDLL_Scatter_CloseHandle(vmsh);

	for (int i = 0; i < ObjectList.size(); i++)
	{
		if (ObjectNameAddresses[i].second != 64)
			continue;

		std::string Name(ObjectNameBuffers[i].Name, ObjectNameBuffers[i].Name + 64);

		if (strcmp("GameWorld", ObjectNameBuffers[i].Name) == 0)
			return ObjectList[i];
	}

	return 0;
}