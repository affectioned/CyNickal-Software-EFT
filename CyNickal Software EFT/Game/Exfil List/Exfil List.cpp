#include "pch.h"

#include "Exfil List.h"
#include "Game/EFT.h"
#include "Game/Offsets/Offsets.h"

void ExfilList::Initialize(DMA_Connection* Conn)
{
	auto LocalGameWorld = EFT::GetCachedWorldAddress();

	auto& Proc = EFT::GetProcess();

	uintptr_t ExfiltrationController = Proc.ReadMem<uintptr_t>(Conn, LocalGameWorld + Offsets::CLocalGameWorld::pExfiltrationController);

	if (!ExfiltrationController)
		throw std::runtime_error("Failed to get Exfiltration Controller");

	uintptr_t ExfilList = Proc.ReadMem<uintptr_t>(Conn, ExfiltrationController + Offsets::CExfiltrationController::pExfiltrationPoints);

	if (!ExfilList)
		throw std::runtime_error("Failed to get Exfiltration Points List");

	uint32_t ExfilCount = Proc.ReadMem<uint32_t>(Conn, ExfilList + Offsets::CGenericList::Num);

	if (ExfilCount > 64)
		throw std::runtime_error("Exfil Count is unreasonably high");

	if (ExfilCount == 0)
		throw std::runtime_error("Exfil Count is zero");

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

void ExfilList::FullUpdate(DMA_Connection* Conn)
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
		Exfil.Finalize();
}