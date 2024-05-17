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
	void KeybindModal(std::string KeybindName, Keybind& keybind);

	/* Set keybind */
	extern Keybind CurrentKeybind;
	extern bool isSettingKeybind;

	/* Keybinds */
	extern Keybind MoveForwardKeybind;
	extern Keybind DodgeKeybind;
	extern Keybind JumpKeybind;
	extern Keybind AboutFaceKeybind;
	extern Keybind DodgeJumpKeybind;
	extern Keybind MoveAboutFaceKeybind;
	extern Keybind HoldDoubleClickKeybind;
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

	static void TooltipGeneric(const char* fmt, ...)
	{
		if (ImGui::Tooltip())
		{
			ImGui::Text(fmt);
			ImGui::EndTooltip();
		}
	}
} // namespace ImGui

#endif /* SETTINGS_H */
