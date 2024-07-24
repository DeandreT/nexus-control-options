#ifndef SETTINGS_H
#define SETTINGS_H

#include <mutex>

#include "imgui/imgui.h"
#include "nlohmann/json.hpp"

#include "Shared.h"

using json = nlohmann::json;

namespace Settings
{
	extern std::mutex Mutex;
	extern std::filesystem::path SettingsPath;
	extern json Settings;

	void Load(const std::filesystem::path& aPath);
	void Save();

	void KeybindButton(std::string keybindName, Keybind& keybind, std::string keybindTooltip);
	void KeybindModal(std::string keybindName, Keybind& keybind);
	void SettingToggle(std::string settingName, bool& setting, std::string settingTooltip);

	void SetDoubleClickModal(std::string modalName);

	/* Set Keybind */
	extern Keybind CurrentKeybind;
	extern bool isSettingKeybind;

	/* Keybinds */
	extern Keybind MoveForwardKeybind;
	extern Keybind DodgeKeybind;
	extern Keybind JumpKeybind;
	extern Keybind AboutFaceKeybind;
	extern Keybind DodgeJumpKeybind;
	extern Keybind MoveAboutFaceKeybind;
	extern Keybind ZoomOutKeybind;
	extern bool AutoAdjustZoomEnabled;
	extern Keybind HoldDoubleClickKeybind;
	extern Keybind SetDoubleClickKeybind;

	/* Set Double-Click */
	extern bool isSettingDoubleClick;
	extern bool isDoubleClickActive;
	extern float doubleClickInterval;
	extern POINT doubleClickCursorPos;
} // namespace Settings

namespace ImGui
{
	static bool Tooltip()
	{
		bool hovered = ImGui::IsItemHovered();
		if (hovered)
		{
			ImGui::BeginTooltip();
		}
		return hovered;
	}

	static void TooltipGeneric(const char* str, ...)
	{
		if (ImGui::Tooltip())
		{
			ImGui::Text(str);
			ImGui::EndTooltip();
		}
	}

	static ImVec2 operator+(const ImVec2& lhs, const ImVec2& rhs)
	{ 
		return ImVec2(lhs.x + rhs.x, lhs.y + rhs.y); 
	}

	static void PaddedText(const char* str, float paddingX, float paddingY)
	{
		ImVec2 textSize = ImGui::CalcTextSize(str);
		ImVec2 cursorStart = ImGui::GetCursorPos();
		ImGui::InvisibleButton("##PaddedText", textSize + ImVec2(paddingX * 2, paddingY * 2));
		ImVec2 cursorFinal = ImGui::GetCursorPos();
		ImGui::SetCursorPos(cursorStart + ImVec2(paddingX, paddingY));
		ImGui::Text(str);
		//ImGui::SetCursorPos(cursorFinal);
	}

} // namespace ImGui

#endif /* SETTINGS_H */
