#include <filesystem>
#include <fstream>

#include "keybinds/Keybinds.h"

#include "Settings.h"

namespace Settings
{
    std::mutex Mutex;
	std::filesystem::path SettingsPath;
    json Settings = json::object();

    void Load(const std::filesystem::path& aPath)
    {
		SettingsPath = aPath;

		Mutex.lock();
		{
			try
			{
				std::ifstream file(SettingsPath);
				Settings = json::parse(file);
				file.close();
			}
			catch (json::parse_error& ex)
			{
				APIDefs->Log(ELogLevel_WARNING, "Extended Control Options", "settings.json could not be parsed.");
				APIDefs->Log(ELogLevel_WARNING, "Extended Control Options", ex.what());
			}
		}
		Mutex.unlock();

		if (!Settings.is_null())
		{
			if (!Settings["MOVE_ABOUT_FACE_KEY"].is_null()) { Settings["MOVE_ABOUT_FACE_KEY"].get_to(MoveAboutFaceKeybind.Key); }
			if (!Settings["MOVE_ABOUT_FACE_ALT"].is_null()) { Settings["MOVE_ABOUT_FACE_ALT"].get_to(MoveAboutFaceKeybind.Alt); }
			if (!Settings["MOVE_ABOUT_FACE_CTRL"].is_null()) { Settings["MOVE_ABOUT_FACE_CTRL"].get_to(MoveAboutFaceKeybind.Ctrl); }
			if (!Settings["MOVE_ABOUT_FACE_SHIFT"].is_null()) { Settings["MOVE_ABOUT_FACE_SHIFT"].get_to(MoveAboutFaceKeybind.Shift); }

			if (!Settings["AUTO_ADJUST_ZOOM_ENABLED"].is_null()) { Settings["AUTO_ADJUST_ZOOM_ENABLED"].get_to(AutoAdjustZoomEnabled); }

			if (!Settings["HOLD_DOUBLE_CLICK_KEY"].is_null()) { Settings["HOLD_DOUBLE_CLICK_KEY"].get_to(HoldDoubleClickKeybind.Key); }
			if (!Settings["HOLD_DOUBLE_CLICK_ALT"].is_null()) { Settings["HOLD_DOUBLE_CLICK_ALT"].get_to(HoldDoubleClickKeybind.Alt); }
			if (!Settings["HOLD_DOUBLE_CLICK_CTRL"].is_null()) { Settings["HOLD_DOUBLE_CLICK_CTRL"].get_to(HoldDoubleClickKeybind.Ctrl); }
			if (!Settings["HOLD_DOUBLE_CLICK_SHIFT"].is_null()) { Settings["HOLD_DOUBLE_CLICK_SHIFT"].get_to(HoldDoubleClickKeybind.Shift); }

			if (!Settings["SET_DOUBLE_CLICK_KEY"].is_null()) { Settings["SET_DOUBLE_CLICK_KEY"].get_to(SetDoubleClickKeybind.Key); }
			if (!Settings["SET_DOUBLE_CLICK_ALT"].is_null()) { Settings["SET_DOUBLE_CLICK_ALT"].get_to(SetDoubleClickKeybind.Alt); }
			if (!Settings["SET_DOUBLE_CLICK_CTRL"].is_null()) { Settings["SET_DOUBLE_CLICK_CTRL"].get_to(SetDoubleClickKeybind.Ctrl); }
			if (!Settings["SET_DOUBLE_CLICK_SHIFT"].is_null()) { Settings["SET_DOUBLE_CLICK_SHIFT"].get_to(SetDoubleClickKeybind.Shift); }
		}
    }

    void Save()
    {
		Settings["MOVE_ABOUT_FACE_KEY"] = MoveAboutFaceKeybind.Key;
		Settings["MOVE_ABOUT_FACE_ALT"] = MoveAboutFaceKeybind.Alt;
		Settings["MOVE_ABOUT_FACE_CTRL"] = MoveAboutFaceKeybind.Ctrl;
		Settings["MOVE_ABOUT_FACE_SHIFT"] = MoveAboutFaceKeybind.Shift;

		Settings["AUTO_ADJUST_ZOOM_ENABLED"] = AutoAdjustZoomEnabled;

		Settings["HOLD_DOUBLE_CLICK_KEY"] = HoldDoubleClickKeybind.Key;
		Settings["HOLD_DOUBLE_CLICK_ALT"] = HoldDoubleClickKeybind.Alt;
		Settings["HOLD_DOUBLE_CLICK_CTRL"] = HoldDoubleClickKeybind.Ctrl;
		Settings["HOLD_DOUBLE_CLICK_SHIFT"] = HoldDoubleClickKeybind.Shift;

		Settings["SET_DOUBLE_CLICK_KEY"] = SetDoubleClickKeybind.Key;
		Settings["SET_DOUBLE_CLICK_ALT"] = SetDoubleClickKeybind.Alt;
		Settings["SET_DOUBLE_CLICK_CTRL"] = SetDoubleClickKeybind.Ctrl;
		Settings["SET_DOUBLE_CLICK_SHIFT"] = SetDoubleClickKeybind.Shift;

		Mutex.lock();
		{
			std::ofstream file(SettingsPath);
			file << Settings.dump(1, '\t') << std::endl;
			file.close();
		}
		Mutex.unlock();
    }

	void KeybindButton(std::string keybindName, Keybind& keybind, std::string keybindTooltip)
	{
		ImGui::TableNextRow();
		ImGui::TableNextColumn();
		ImGui::PaddedText(keybindName.c_str(), 6.0F, 4.0F);
		ImGui::TableNextColumn();
		if (ImGui::Button(std::string(Keybinds::KeybindToString(keybind) + "##" + keybindName).c_str()))
		{
			ImGui::OpenPopup(std::string("Set Keybind: " + keybindName).c_str(), ImGuiPopupFlags_AnyPopupLevel);
		}
		ImGui::TooltipGeneric(keybindTooltip.c_str());
		ImGui::TableNextColumn();

		KeybindModal(keybindName, keybind);
	}

	void KeybindModal(std::string keybindName, Keybind& keybind)
	{
		if (ImGui::BeginPopupModal(std::string("Set Keybind: " + keybindName).c_str()))
		{
			bool closeModal = false;
			isSettingKeybind = true;

			if (CurrentKeybind == Keybind{})
			{
				CurrentKeybind = keybind;
			}
			ImGui::Text(Keybinds::KeybindToString(CurrentKeybind).c_str());

			if (ImGui::Button("Unbind"))
			{
				keybind = {};
				closeModal = true;
			}

			ImGui::SameLine(); ImGui::Spacing(); 
			ImGui::SameLine(); ImGui::Spacing(); 
			ImGui::SameLine(); ImGui::Spacing();

			ImGui::SameLine();
			if (ImGui::Button("Accept"))
			{
				keybind = CurrentKeybind;
				closeModal = true;
			}
			
			ImGui::SameLine();
			if (ImGui::Button("Cancel"))
			{
				closeModal = true;
			}

			if (closeModal)
			{
				CurrentKeybind = Keybind{};
				isSettingKeybind = false;
				Save();
				ImGui::CloseCurrentPopup();
			}

			ImGui::EndPopup();
		}
	}

	void SettingToggle(std::string settingName, bool& setting, std::string settingTooltip)
	{
		ImGui::TableNextRow();
		ImGui::TableNextColumn();
		ImGui::PaddedText("Auto-Adjust Zoom", 6.0F, 4.0F);
		ImGui::TableNextColumn();
		if (ImGui::Checkbox(std::string("Enabled##" + settingName).c_str(), &setting))
		{
			Save();
		}
		ImGui::TooltipGeneric(settingTooltip.c_str());
		ImGui::TableNextColumn();
	}

	void SetDoubleClickModal(std::string modalName)
	{
		if (ImGui::BeginPopupModal(modalName.c_str()))
		{
			bool closeModal = false;

			if (!isSettingDoubleClick)
			{
				GetCursorPos(&doubleClickCursorPos);
			}

			isSettingDoubleClick = true;
			isDoubleClickActive = false;

			ImGui::InputFloat(std::string("seconds##" + modalName).c_str(), &doubleClickInterval, 0.25F, 0.25F, "%.2f");

			if (doubleClickInterval < 0.0F)
			{
				doubleClickInterval = 0.0F;
			}

			if (ImGui::Button(std::string("Start##" + modalName).c_str()))
			{
				isDoubleClickActive = true;
				closeModal = true;
			}

			ImGui::SameLine(); ImGui::Spacing(); ImGui::SameLine(); ImGui::Spacing();
			ImGui::SameLine(); ImGui::Spacing(); ImGui::SameLine(); ImGui::Spacing();
			ImGui::SameLine(); ImGui::Spacing(); ImGui::SameLine(); ImGui::Spacing();
			ImGui::SameLine(); ImGui::Spacing(); ImGui::SameLine(); ImGui::Spacing();
			ImGui::SameLine(); ImGui::Spacing(); ImGui::SameLine(); ImGui::Spacing();
			ImGui::SameLine(); ImGui::Spacing(); ImGui::SameLine(); ImGui::Spacing();
			ImGui::SameLine(); ImGui::Spacing(); ImGui::SameLine(); ImGui::Spacing();
			ImGui::SameLine(); ImGui::Spacing(); ImGui::SameLine(); ImGui::Spacing();

			ImGui::SameLine();
			if (ImGui::Button(std::string("Cancel##" + modalName).c_str()))
			{
				// set back to default value
				doubleClickInterval = 0.75F;
				closeModal = true;
			}

			if (closeModal)
			{
				isSettingDoubleClick = false;
				ImGui::CloseCurrentPopup();
			}

			ImGui::EndPopup();
		}
	}

	/* Set Keybind */
	Keybind CurrentKeybind {};
	bool isSettingKeybind = false;

	/* Keybinds & Settings */
	Keybind MoveForwardKeybind {};
	Keybind AboutFaceKeybind {};
	Keybind MoveAboutFaceKeybind {};
	Keybind ZoomOutKeybind {};
	bool AutoAdjustZoomEnabled = false;
	Keybind HoldDoubleClickKeybind {};
	Keybind SetDoubleClickKeybind {};
	
	/* Toggle Double-Click */
	bool isSettingDoubleClick = false;
	bool isDoubleClickActive = false;
	float doubleClickInterval = 0.75F;
	POINT doubleClickCursorPos = { 0 };
} // namespace Settings
