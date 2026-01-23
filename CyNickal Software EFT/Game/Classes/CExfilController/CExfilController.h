#pragma once
#include "DMA/DMA.h"
#include "Game/Classes/CExfilPoint/CExfilPoint.h"
#include "Game/Classes/CBaseEntity/CBaseEntity.h"

class CExfilController : public CBaseEntity
{
public:
	CExfilController(uintptr_t ExfilControllersAddress, uintptr_t MapNameAddress);
private:
	void ReadMapName(DMA_Connection* Conn, uintptr_t MapNameAddress);
	void Initialize(DMA_Connection* Conn);
	void FullUpdate(DMA_Connection* Conn);

public:
	std::mutex m_ExfilMutex{};
	std::vector<CExfilPoint> m_Exfils{};
	std::string m_MapName;
};
