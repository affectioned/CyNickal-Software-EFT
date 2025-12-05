#include "pch.h"
#include "GUI/Fuser/Draw/Players.h"
#include "Game/Camera/Camera.h"
#include "GUI/Color Picker/Color Picker.h"
#include "Players.h"

void DrawTextAtPosition(ImDrawList* DrawList, const ImVec2& Position, const ImColor& Color, const std::string& Text)
{
	auto TextSize = ImGui::CalcTextSize(Text.c_str());
	DrawList->AddText(
		ImVec2(Position.x - (TextSize.x / 2.0f), Position.y),
		Color,
		Text.c_str()
	);
}

static const std::string PlayerLabel = "Player";
static const std::string ScavLabel = "Scav";
void DrawESPPlayers::Draw(const CClientPlayer& Player, const ImVec2& WindowPos, ImDrawList* DrawList)
{
	if (Player.IsInvalid())	return;

	Vector2 ScreenPos{};
	if (!Camera::WorldToScreen(Player.m_RootPosition, ScreenPos)) return;

	DrawTextAtPosition(
		DrawList,
		ImVec2(WindowPos.x + ScreenPos.x, WindowPos.y + ScreenPos.y),
		Player.IsAi() ? ColorPicker::m_ScavColor : ColorPicker::m_EnemyColor,
		Player.IsAi() ? ScavLabel : PlayerLabel
		);
}

void DrawESPPlayers::Draw(const CObservedPlayer& Player, const ImVec2& WindowPos, ImDrawList* DrawList)
{
	if (Player.IsInvalid())	return;

	Vector2 ScreenPos{};
	if (!Camera::WorldToScreen(Player.m_RootPosition, ScreenPos)) return;

	DrawTextAtPosition(
		DrawList,
		ImVec2(WindowPos.x + ScreenPos.x, WindowPos.y + ScreenPos.y),
		Player.IsAi() ? ColorPicker::m_ScavColor : ColorPicker::m_EnemyColor,
		Player.IsAi() ? ScavLabel : PlayerLabel
	);
}