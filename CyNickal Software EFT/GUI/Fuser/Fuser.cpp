#include "pch.h"
#include "Fuser.h"
#include "Game/Camera/Camera.h"
#include "Game/Player List/Player List.h"

void Fuser::Render()
{
	ImGui::SetNextWindowSize({ 1920,1080 });
	ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.0f, 0.0f, 0.0f, 255.0f));
	ImGui::Begin("Fuser", nullptr, ImGuiWindowFlags_NoDecoration);
	std::scoped_lock lk(PlayerList::m_PlayerMutex);
	for (auto& Player : PlayerList::m_Players)
	{
		Vector2 ScreenPosition{};
		if (Camera::WorldToScreen(Player.m_BasePosition, ScreenPosition))
		{
			auto WindowPos = ImGui::GetWindowPos();
			ImGui::GetWindowDrawList()->AddCircleFilled({ WindowPos.x + ScreenPosition.x, WindowPos.y + ScreenPosition.y }, 5.0f, IM_COL32(255, 0, 0, 255));
		}
	}
	ImGui::End();
	ImGui::PopStyleColor();
}