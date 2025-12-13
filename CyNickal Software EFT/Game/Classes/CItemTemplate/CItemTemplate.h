#pragma once

#include "Game/Classes/CBaseEntity/CBaseEntity.h"
#include "Game/Classes/CNameHash/CNameHash.h"

class CItemTemplate : public CBaseEntity
{
public:
	CItemTemplate(uintptr_t TemplateAddress);
	void PrepareRead_1(VMMDLL_SCATTER_HANDLE vmsh);
	void PrepareRead_2(VMMDLL_SCATTER_HANDLE vmsh);
	void Finalize();

public:
	std::wstring m_wName{ L"" };
	std::string m_sName{ "" };
	std::unique_ptr<CNameHash> m_pNameHash{ nullptr };

private:
	uintptr_t m_NameAddress{ 0 };
};