#include "pch.h"	
#include "CExfilController.h"
#include "Game/EFT.h"
#include "Game/Offsets/Offsets.h"

CExfilController::CExfilController(uintptr_t ExfilControllersAddress, uintptr_t MapNameAddress) : CBaseEntity(ExfilControllersAddress)
{
	std::println("[CExfilController] Constructed with {0:X} and MapNameAddress {0:X}", m_EntityAddress, MapNameAddress);

	auto Conn = DMA_Connection::GetInstance();

	ReadMapName(Conn, MapNameAddress);

	Initialize(Conn);
}

// could be move inside Initialize(DMA_Connection* Conn)
void CExfilController::ReadMapName(DMA_Connection* Conn, uintptr_t MapNameAddress)
{
	if (MapNameAddress == 0)
		return;
	auto& Proc = EFT::GetProcess();

	// IL2CPP String structure:
	// +0x00: object header
	// +0x10: int32 length
	// +0x14: wchar_t data[length]

	// Read the string length
	int32_t length = Proc.ReadMem<int32_t>(Conn, MapNameAddress + 0x10);

	if (length <= 0 || length > 256)
		return;

	std::vector<wchar_t> wideBuffer = Proc.ReadVec<wchar_t>(Conn, MapNameAddress + 0x14, length);

	std::wstring wideStr(wideBuffer.data(), length);
	m_MapName = std::string(wideStr.begin(), wideStr.end());

	std::println("[CExfilController] Map Name: {}", m_MapName);
}

void CExfilController::Initialize(DMA_Connection* Conn)
{
	auto& Proc = EFT::GetProcess();

	uintptr_t ExfilList = Proc.ReadMem<uintptr_t>(Conn, m_EntityAddress + Offsets::CExfiltrationController::pExfiltrationPoints);

	if (!ExfilList)
	{
		std::println("[CExfilController] ExfilList is null");
		SetInvalid();
		return;
	}

	uint32_t ExfilCount = Proc.ReadMem<uint32_t>(Conn, ExfilList + Offsets::CGenericList::Num);

	if (ExfilCount > 64)
	{
		std::println("[CExfilController] Number of exfils is unreasonably high: {}", ExfilCount);
		SetInvalid();
		return;
	}

	if (ExfilCount == 0)
	{
		std::println("[CExfilController] Exfil Count is zero");
		SetInvalid();
		return;
	}

	uintptr_t ExfilDataStart = ExfilList + Offsets::CGenericList::StartData;

	auto ExfilPointers = Proc.ReadVec<uintptr_t>(Conn, ExfilDataStart, ExfilCount);

	{
		std::scoped_lock Lock(m_ExfilMutex);
		m_Exfils.clear();

		for (const auto& ExfilPtr : ExfilPointers)
			m_Exfils.emplace_back(CExfilPoint(ExfilPtr));
	}

	FullUpdate(Conn);
}

void CExfilController::FullUpdate(DMA_Connection* Conn)
{
	std::scoped_lock Lock(m_ExfilMutex);

	auto PID = EFT::GetProcess().GetPID();

	auto vmsh = VMMDLL_Scatter_Initialize(Conn->GetHandle(), PID, VMMDLL_FLAG_NOCACHE);

	for (auto& Exfil : m_Exfils)
		Exfil.PrepareRead_1(vmsh);
	VMMDLL_Scatter_Execute(vmsh);
	VMMDLL_Scatter_Clear(vmsh, PID, VMMDLL_FLAG_NOCACHE);

	for (auto& Exfil : m_Exfils)
		Exfil.PrepareRead_2(vmsh);
	VMMDLL_Scatter_Execute(vmsh);
	VMMDLL_Scatter_Clear(vmsh, PID, VMMDLL_FLAG_NOCACHE);

	for (auto& Exfil : m_Exfils)
		Exfil.PrepareRead_3(vmsh);
	VMMDLL_Scatter_Execute(vmsh);
	VMMDLL_Scatter_Clear(vmsh, PID, VMMDLL_FLAG_NOCACHE);

	for (auto& Exfil : m_Exfils)
		Exfil.PrepareRead_4(vmsh);
	VMMDLL_Scatter_Execute(vmsh);
	VMMDLL_Scatter_Clear(vmsh, PID, VMMDLL_FLAG_NOCACHE);

	for (auto& Exfil : m_Exfils)
		Exfil.PrepareRead_5(vmsh);
	VMMDLL_Scatter_Execute(vmsh);
	VMMDLL_Scatter_Clear(vmsh, PID, VMMDLL_FLAG_NOCACHE);

	for (auto& Exfil : m_Exfils)
		Exfil.PrepareRead_6(vmsh);
	VMMDLL_Scatter_Execute(vmsh);
	VMMDLL_Scatter_Clear(vmsh, PID, VMMDLL_FLAG_NOCACHE);

	for (auto& Exfil : m_Exfils)
		Exfil.PrepareRead_7(vmsh);
	VMMDLL_Scatter_Execute(vmsh);
	VMMDLL_Scatter_Clear(vmsh, PID, VMMDLL_FLAG_NOCACHE);

	for (auto& Exfil : m_Exfils)
		Exfil.PrepareRead_8(vmsh);
	VMMDLL_Scatter_Execute(vmsh);
	VMMDLL_Scatter_CloseHandle(vmsh);

	for (auto& Exfil : m_Exfils)
		Exfil.Finalize(m_MapName);
}