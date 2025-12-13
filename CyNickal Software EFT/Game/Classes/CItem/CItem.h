#pragma once
#include "Game/Classes/CBaseEntity/CBaseEntity.h"
#include "Game/Classes/CItemTemplate/CItemTemplate.h"

class CItem : public CBaseEntity
{
public:
	CItem(uintptr_t EntityAddress);
	void PrepareRead_1(VMMDLL_SCATTER_HANDLE vmsh);
	void PrepareRead_2(VMMDLL_SCATTER_HANDLE vmsh);
	void PrepareRead_3(VMMDLL_SCATTER_HANDLE vmsh);
	void CompleteUpdate();
	void Finalize();
	const char* GetUnfilteredName() const;
	const char* GetSanitizedName() const;

public:
	std::unique_ptr<CItemTemplate> m_pItemTemplate{ nullptr };

private:
	uintptr_t m_ItemTemplateAddress{ 0 };
};