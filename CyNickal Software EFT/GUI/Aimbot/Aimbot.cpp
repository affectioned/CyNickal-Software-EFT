#include "pch.h"
#include "Aimbot.h"
#include "DMA/Input Manager.h"
#include "Game/Camera List/Camera List.h"
#include "GUI/Fuser/Fuser.h"
#include "GUI/Keybinds/Keybinds.h"	
#include "Game/EFT.h"
#include "Makcu/MyMakcu.h"

void Aimbot::RenderSettings()
{
	if (!bSettings) return;

	ImGui::Begin("Aimbot Settings", &bSettings);
	ImGui::Checkbox("Master Toggle", &bMasterToggle);
	ImGui::Checkbox("Draw FOV Circle", &bDrawFOV);
	ImGui::SliderFloat("Alpha X", &fAlphaX, 0.001f, 1.0f, "%.3f", ImGuiSliderFlags_Logarithmic);
	ImGui::SliderFloat("Alpha Y", &fAlphaY, 0.001f, 1.0f, "%.3f", ImGuiSliderFlags_Logarithmic);
	ImGui::SliderFloat("Gaussian Noise", &fGaussianNoise, 0.0f, 5.0f, "%.2f");
	ImGui::SliderFloat("Prediction", &fPrediction, 0.0f, 2.0f, "%.2f");
	ImGui::SliderFloat("FOV", &fPixelFOV, 1.0f, 300.0f);
	ImGui::SliderFloat("Deadzone FOV", &fDeadzoneFov, 1.0f, 10.0f);

	ImGui::End();
}

void Aimbot::RenderFOVCircle(const ImVec2& WindowPos, ImDrawList* DrawList)
{
	if (!bMasterToggle || !bDrawFOV) return;

	auto WindowSize = ImGui::GetWindowSize();
	auto Center = ImVec2(WindowPos.x + WindowSize.x / 2.0f, WindowPos.y + WindowSize.y / 2.0f);
	DrawList->AddCircle(Center, fPixelFOV, IM_COL32(255, 255, 255, 255), 100, 2.0f);
	DrawList->AddCircle(Center, fDeadzoneFov, IM_COL32(255, 0, 0, 255), 100, 2.0f);
}

ImVec2 Subtract(const ImVec2& lhs, const ImVec2& rhs)
{
	return { lhs.x - rhs.x, lhs.y - rhs.y };
}
ImVec2 Subtract(const Vector2& lhs, const ImVec2& rhs)
{
	return { lhs.x - rhs.x, lhs.y - rhs.y };
}
float Distance(Vector2 a, ImVec2 b)
{
	return sqrtf(powf(b.x - a.x, 2) + powf(b.y - a.y, 2));
}

void Aimbot::OnDMAFrame(DMA_Connection* Conn)
{
	if (!bMasterToggle) return;
	if (c_keys::IsInitialized() == false || MyMakcu::m_Device.isConnected() == false) return;
	if (Keybinds::Aimbot.IsActive(Conn) == false) return;

	auto BestTarget = Aimbot::FindBestTarget();
	auto& RegisteredPlayers = EFT::GetRegisteredPlayers();

	auto LastTime = std::chrono::steady_clock::time_point();

	do
	{
		auto CurrentTime = std::chrono::high_resolution_clock::now();
		auto DeltaTime = std::chrono::duration_cast<std::chrono::milliseconds>(CurrentTime - LastTime).count();
		if (DeltaTime < 5) continue;
		LastTime = CurrentTime;

		RegisteredPlayers.QuickUpdate(Conn);
		CameraList::QuickUpdateNecessaryCameras(Conn);

		auto Delta = GetAimDeltaToTarget(BestTarget);

		if (Delta.x == 0.0f && Delta.y == 0.0f)
			continue;

		// Lerp from no movement toward full delta — lower alpha = smoother
		Vector2 MoveAmount{
			std::lerp(0.0f, Delta.x, fAlphaX),
			std::lerp(0.0f, Delta.y, fAlphaY)
		};

		std::normal_distribution<float> noise(0.0f, fGaussianNoise);
		MoveAmount.x += noise(gen);
		MoveAmount.y += noise(gen);

		MyMakcu::m_Device.mouseMove(MoveAmount.x, MoveAmount.y);

	} while (Keybinds::Aimbot.IsActive(Conn));
}

ImVec2 Aimbot::GetAimDeltaToTarget(uintptr_t TargetAddress)
{
	ImVec2 Return{};

	if (TargetAddress == 0x0) return Return;

	auto CenterScreen = Fuser::GetCenterScreen();

	auto TargetWorldPos = EFT::GetRegisteredPlayers().GetPlayerBonePosition(TargetAddress, EBoneIndex::Head);

	Vector2 ScreenPos{};
	if (!CameraList::W2S(TargetWorldPos, ScreenPos)) return Return;

	float DistanceFromCenter = Distance(ScreenPos, CenterScreen);

	if (DistanceFromCenter < fDeadzoneFov) return Return;

	if (DistanceFromCenter > fPixelFOV) return Return;

	// Extrapolate screen position to compensate for lerp lag on moving targets.
	// fPrediction is a fraction of the full lag compensation (1.0 = no lag at all).
	// Internally scaled by 1/alpha since steady-state lag = velocity / alpha.
	// On target switch, skip prediction for one frame to avoid a jump.
	Vector2 PredictedPos = ScreenPos;
	if (fPrediction > 0.0f && TargetAddress == m_LastTargetAddress)
	{
		PredictedPos.x += (ScreenPos.x - m_LastScreenPos.x) * (fPrediction / fAlphaX);
		PredictedPos.y += (ScreenPos.y - m_LastScreenPos.y) * (fPrediction / fAlphaY);
	}

	m_LastScreenPos = ScreenPos;
	m_LastTargetAddress = TargetAddress;

	Return = Subtract(PredictedPos, CenterScreen);

	return Return;
}

uintptr_t Aimbot::FindBestTarget()
{
	auto& PlayerList = EFT::GetRegisteredPlayers();

	std::scoped_lock lk(PlayerList.m_Mut);

	auto Center = Fuser::GetCenterScreen();
	uintptr_t BestTarget = 0;
	float BestDistance = std::numeric_limits<float>::max();

	for (auto& Player : PlayerList.m_Players)
	{
		std::visit([&](auto& Player) {

			Vector2 ScreenPos{};
			if (!CameraList::W2S(Player.GetBonePosition(EBoneIndex::Head), ScreenPos)) return;

			float DistanceFromCenter = sqrt(pow(ScreenPos.x - Center.x, 2) + pow(ScreenPos.y - Center.y, 2));

			if (DistanceFromCenter > fPixelFOV) return;

			if (DistanceFromCenter < BestDistance)
			{
				BestTarget = Player.m_EntityAddress;
				BestDistance = DistanceFromCenter;
			}

			}, Player);
	}

	return BestTarget;
}