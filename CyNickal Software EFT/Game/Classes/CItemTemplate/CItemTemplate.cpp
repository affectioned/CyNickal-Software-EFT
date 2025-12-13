#include "pch.h"
#include "CItemTemplate.h"
#include "Game/Offsets/Offsets.h"

CItemTemplate::CItemTemplate(uintptr_t TemplateAddress)
	: CBaseEntity(TemplateAddress)
{
	std::println("[CItemTemplate] Constructed at {:X}", TemplateAddress);

	m_wName.resize(64);
}

void CItemTemplate::PrepareRead_1(VMMDLL_SCATTER_HANDLE vmsh)
{
	VMMDLL_Scatter_PrepareEx(vmsh, m_EntityAddress + Offsets::CItemTemplate::pName, sizeof(uintptr_t), reinterpret_cast<BYTE*>(&m_NameAddress), reinterpret_cast<DWORD*>(&m_BytesRead));
}

void CItemTemplate::PrepareRead_2(VMMDLL_SCATTER_HANDLE vmsh)
{
	if (m_BytesRead != sizeof(uintptr_t))
		SetInvalid();

	if (!m_NameAddress)
		SetInvalid();

	if (IsInvalid()) return;

	VMMDLL_Scatter_PrepareEx(vmsh, m_NameAddress + 0x14, m_wName.size() * sizeof(wchar_t), reinterpret_cast<BYTE*>(m_wName.data()), reinterpret_cast<DWORD*>(&m_BytesRead));
}

void CItemTemplate::Finalize()
{
	if (m_BytesRead != m_wName.size() * sizeof(wchar_t))
		SetInvalid();

	if (IsInvalid()) return;

	m_sName = std::string(m_wName.begin(), m_wName.end());
	m_pNameHash = std::make_unique<CNameHash>(m_sName);

	std::println("[CItemTemplate] Finalized Template at {0:X} with Name: {1:s}", m_EntityAddress, m_sName.c_str());
}