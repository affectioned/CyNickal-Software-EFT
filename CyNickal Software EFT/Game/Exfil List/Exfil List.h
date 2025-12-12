#pragma once
#include "DMA/DMA.h"
#include "Game/Classes/CExfilPoint/CExfilPoint.h"

class ExfilList
{
public:
	static void Initialize(DMA_Connection* Conn);
	static void FullUpdate(DMA_Connection* Conn);

public:
	static inline std::mutex m_ExfilMutex{};
	static inline std::vector<CExfilPoint> m_Exfils{};
};