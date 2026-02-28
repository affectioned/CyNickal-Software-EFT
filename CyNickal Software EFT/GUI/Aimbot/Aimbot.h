#pragma once
#include "DMA/DMA.h"

class Aimbot
{
public:
	static void RenderSettings();
	static void RenderFOVCircle(const ImVec2& WindowPos, ImDrawList* DrawList);
	static void OnDMAFrame(DMA_Connection* Conn);

public:
	static inline bool bSettings{ false };
	static inline bool bMasterToggle{ false };
	static inline bool bDrawFOV{ true };
	static inline float fAlphaX{ 0.03f };
	static inline float fAlphaY{ 0.03f };
	static inline float fGaussianNoise{ 0.5f };
	static inline float fPixelFOV{ 75.0f };
	static inline float fDeadzoneFov{ 2.0f };

private:
	static inline std::mt19937 gen{ std::random_device{}() };

private:
	static ImVec2 GetAimDeltaToTarget(uintptr_t TargetAddress);
	static uintptr_t FindBestTarget();
};