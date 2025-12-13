#include "pch.h"
#include "Ammo Count.h"
#include "Game/Player List/Player List.h"

void AmmoCountOverlay::Render()
{
	auto WindowSize = ImGui::GetWindowSize();
	auto WindowPos = ImGui::GetWindowPos();

	std::pair<uint32_t, uint32_t> AmmoCountPair = GetLocalPlayerAmmoCount();
	if (AmmoCountPair.second == 0)
		return;

	ImGui::PushFont(nullptr, 32.0f);
	std::string AmmoString = std::format("{}/{}", AmmoCountPair.first, AmmoCountPair.second);
	auto TextSize = ImGui::CalcTextSize(AmmoString.c_str());

	ImGui::SetNextWindowPos({ WindowPos.x + (WindowSize.x * 0.5f) - (TextSize.x * 0.5f), WindowPos.y + WindowSize.y - TextSize.y });
	ImGui::SetNextWindowSize({ TextSize.x , TextSize.y });
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, { 4.0f,0.0f });
	ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
	ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2());
	ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, ImVec2());
	ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2());
	ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 10.0f);
	if (ImGui::BeginChild("##AmmoCountOverlay", {}, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoScrollbar))
	{
		ImGui::TextColored(ImVec4(1.0f, 1.0f, 1.0f, 1.0f), AmmoString.c_str());
	}
	ImGui::EndChild();
	ImGui::PopStyleVar(6);
	ImGui::PopFont();
}

std::pair<uint32_t, uint32_t> AmmoCountOverlay::GetLocalPlayerAmmoCount()
{
	std::scoped_lock Lock(PlayerList::m_PlayerMutex);
	auto LocalPlayer = PlayerList::GetLocalPlayer();

	return GetAmmoCount(LocalPlayer);
}

std::pair<uint32_t, uint32_t> AmmoCountOverlay::GetAmmoCount(CBaseEFTPlayer* Player)
{
	auto ReturnValue = std::pair<uint32_t, uint32_t>();

	if (!Player) return ReturnValue;
	if (Player->IsInvalid()) return ReturnValue;
	if (Player->m_pHands == nullptr) return ReturnValue;
	if (Player->m_pHands->m_pMagazine == nullptr) return ReturnValue;

	auto& Magazine = Player->m_pHands->m_pMagazine;

	ReturnValue.first = Magazine->m_CurrentCartridges;
	ReturnValue.second = Magazine->m_MaxCartridges;

	return ReturnValue;
}