#pragma once
#include "DMA/DMA.h"
#include "Game/Classes/Vector.h"

struct Matrix44
{
	float M[4][4]{};
};

struct CCameraEntry
{
	uintptr_t Address{ 0 };
	std::string Name{};
};

class Camera
{
public:
	static bool Initialize(DMA_Connection* Conn);
	static bool WorldToScreen(Vector3 WorldPosition, Vector2& ScreenPosition);

private:
	static inline uintptr_t m_CameraHeadAddress{ 0 };
	static inline uint32_t m_NumCameras{ 0 };
	static inline std::vector<CCameraEntry> m_CameraCache{};

	static inline std::mutex m_MatrixMutex{};
	static inline uintptr_t m_ViewMatrixAddress{ 0 };
	static inline Matrix44 m_ViewMatrix{};

public:
	static void QuickUpdateViewMatrix(DMA_Connection* Conn);
	static bool InitializeCameraCache(DMA_Connection* Conn);
	static uintptr_t SearchCameraCacheByName(const std::string& Name);
	static Matrix44 GetViewMatrix();
};