#pragma once
#include "DMA/DMA.h"
#include "Game/Classes/Vector.h"

class Aimbot
{
public:
	static void RenderSettings();
	static void RenderFOVCircle(const ImVec2& WindowPos, ImDrawList* DrawList);
	static void OnDMAFrame(DMA_Connection* Conn);

public:
	static inline bool bSettings{ true };
	static inline bool bMasterToggle{ false };
	static inline bool bDrawFOV{ true };
	static inline float fAlphaX{ 0.07f };
	static inline float fAlphaY{ 0.07f };
	static inline float fGaussianNoise{ 0.5f };
	static inline float fPrediction{ 1.0f };
	static inline float fPixelFOV{ 75.0f };
	static inline float fDeadzoneFov{ 2.0f };

private:
	static inline std::mt19937 gen{ std::random_device{}() };
	static inline Vector2 m_LastScreenPos{};
	static inline uintptr_t m_LastTargetAddress{ 0 };

private:
	static ImVec2 GetAimDeltaToTarget(uintptr_t TargetAddress);
	static uintptr_t FindBestTarget();
};