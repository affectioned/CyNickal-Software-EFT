#pragma once
#include "DMA/DMA.h"
#include "Game/Classes/Vector.h"
#include "Game/Classes/CCamera/CCamera.h"

class CameraList
{
public:
	static bool Initialize(DMA_Connection* Conn);
	static bool W2S(const Vector3 WorldPosition, Vector2& ScreenPosition);
	static bool OpticW2S(const Vector3 WorldPosition, Vector2& ScreenPosition);
	static CCamera* GetActiveOptic();
	static bool IsScoped();

	static void QuickUpdateNecessaryCameras(DMA_Connection* Conn);
	static float GetOpticRadius();
	static Vector2 GetOpticCenter();

private:
	static inline std::mutex m_CacheMutex{};
	static inline std::vector<CCamera> m_CameraCache{};
	static inline CCamera* m_pFPSCamera{ nullptr };
	static inline std::vector<CCamera*> m_pOpticCameras{};
	static inline CCamera* m_pActiveOpticCamera{ nullptr };

private:
	static bool WorldToScreenEx(const Vector3 WorldPosition, Vector2& ScreenPosition, CCamera* FPSCamera, CCamera* OpticCamera = nullptr);
	static bool CreateCameraCache(DMA_Connection* Conn, uintptr_t CameraHeadAddress, uint32_t NumCameras);
	static CCamera* SearchCameraCacheByName(const std::string& Name);
	static std::vector<CCamera*> GetPotentialOpticCameras();
	static CCamera* FindWinningOptic(const std::vector<CCamera*>& PotentialOpticCams);
};