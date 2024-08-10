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

	void ToggleDoubleClickModal(std::string modalName);

	/* Set Keybind */
	extern Keybind CurrentKeybind;
	extern bool isSettingKeybind;

	/* Settings */
	extern bool AutoAdjustZoomFOV;
	extern bool AutoAdjustZoomMap;
	extern bool ManualAdjustZoom;

	/* Toggle Double-Click */
	extern bool isDoubleClickActive;
	extern bool isDoubleClickPosFixed;
	extern bool isSettingDoubleClick;
	extern std::string doubleClickKeybindId;
	extern UINT doubleClickTexId;
	extern float doubleClickInterval;
	extern POINT doubleClickCursorPos;
} // namespace Settings

namespace ImGui
{
	static ImVec2 operator+(const ImVec2& lhs, const ImVec2& rhs)
	{
		return ImVec2(lhs.x + rhs.x, lhs.y + rhs.y);
	}
	
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

	static void TextPadded(const char* str, float paddingX, float paddingY)
	{
		ImVec2 textSize = ImGui::CalcTextSize(str);
		ImVec2 cursorStart = ImGui::GetCursorPos();
		ImGui::InvisibleButton("##TextPadded", textSize + ImVec2(paddingX * 2, paddingY * 2));
		ImVec2 cursorFinal = ImGui::GetCursorPos();
		ImGui::SetCursorPos(cursorStart + ImVec2(paddingX, paddingY));
		ImGui::Text(str);
	}

	static void TextWrappedPadded(const char* str, float paddingX, float paddingY)
	{
		ImVec2 textSize = ImGui::CalcTextSize(str);
		ImVec2 cursorStart = ImGui::GetCursorPos();
		ImGui::InvisibleButton("##TextWrappedPadded", textSize + ImVec2(paddingX * 2, paddingY * 2));
		ImVec2 cursorFinal = ImGui::GetCursorPos();
		ImGui::SetCursorPos(cursorStart + ImVec2(paddingX, paddingY));
		ImGui::TextWrapped(str);
	}

	static bool SelectablePadded(const char* label, bool selected, float paddingX, float paddingY)
	{
		return ImGui::Selectable(label, selected);
	}

} // namespace ImGui

#endif /* SETTINGS_H */
