#pragma once

#include "DMA/DMA.h"
#include "DMA/Process.h"

#include "Classes/CObjectInfo.h"
class EFT
{
public:
	static bool Initialize(DMA_Connection* Conn);
	static const Process& GetProcess();
	static uintptr_t GetCachedWorldAddress();

private:
	static inline Process Proc{};
	static inline uintptr_t m_CachedLocalWorldAddress{ 0 };
};