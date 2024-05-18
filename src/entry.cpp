#include <Windows.h>
#include <filesystem>
#include <fstream>
#include <mutex>
#include <vector>

#include "nexus/Nexus.h"
#include "mumble/Mumble.h"
#include "keybinds/Keybinds.h"
#include "imgui/imgui.h"

#include "Shared.h"
#include "Settings.h"

void OnMumbleIdentityUpdated(void* aEventArgs);
void AddonLoad(AddonAPI* aApi);
void AddonUnload();
UINT AddonWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
void AddonRender();
void AddonOptions();

AddonDefinition AddonDef = {};
HWND hGame = nullptr;
HMODULE hSelf = nullptr;

BOOL APIENTRY DllMain(HMODULE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved)
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH: 
		hSelf = hModule; 
		break;
	case DLL_PROCESS_DETACH: 
		break;
	case DLL_THREAD_ATTACH: 
		break;
	case DLL_THREAD_DETACH: 
		break;
	}
	return TRUE;
}

extern "C" __declspec(dllexport) AddonDefinition * GetAddonDef()
{
	AddonDef.Signature = 37;
	AddonDef.APIVersion = NEXUS_API_VERSION;
	AddonDef.Name = "Extended Control Options";
	AddonDef.Version.Major = 2024;
	AddonDef.Version.Minor = 5;
	AddonDef.Version.Build = 18;
	AddonDef.Version.Revision = 1;
	AddonDef.Author = "Jordan";
	AddonDef.Description = "Provides additional controls and macros not available via the Control Options menu.";
	AddonDef.Load = AddonLoad;
	AddonDef.Unload = AddonUnload;
	AddonDef.Flags = EAddonFlags_None;

	/* not necessary if hosted on Raidcore, but shown anyway for the example also useful as a backup resource */
	AddonDef.Provider = EUpdateProvider_GitHub;
	AddonDef.UpdateLink = "https://github.com/jordanrye/nexus-control-options";

	return &AddonDef;
}

void OnMumbleIdentityUpdated(void* aEventArgs)
{
	MumbleIdentity = (Mumble::Identity*)aEventArgs;
}

void AddonLoad(AddonAPI* aApi)
{
	APIDefs = aApi;
	ImGui::SetCurrentContext((ImGuiContext*)APIDefs->ImguiContext);
	ImGui::SetAllocatorFunctions((void* (*)(size_t, void*))APIDefs->ImguiMalloc, (void(*)(void*, void*))APIDefs->ImguiFree); // on imgui 1.80+

	NexusLink = (NexusLinkData*)APIDefs->GetResource("DL_NEXUS_LINK");
	MumbleLink = (Mumble::Data*)APIDefs->GetResource("DL_MUMBLE_LINK");

	APIDefs->SubscribeEvent("EV_MUMBLE_IDENTITY_UPDATED", OnMumbleIdentityUpdated);

	APIDefs->RegisterRender(ERenderType_Render, AddonRender);
	APIDefs->RegisterRender(ERenderType_OptionsRender, AddonOptions);

	APIDefs->RegisterWndProc(AddonWndProc);

	std::filesystem::create_directory(APIDefs->GetAddonDirectory("ControlOptions"));
	Settings::Load(APIDefs->GetAddonDirectory("ControlOptions/settings.json"));
}
void AddonUnload()
{
	APIDefs->DeregisterWndProc(AddonWndProc);

	APIDefs->DeregisterRender(AddonOptions);
	APIDefs->DeregisterRender(AddonRender);

	APIDefs->UnsubscribeEvent("EV_MUMBLE_IDENTITY_UPDATED", OnMumbleIdentityUpdated);

	MumbleLink = nullptr;
	NexusLink = nullptr;

	Settings::Save();
}

bool isDodgeJumpPressed = false;
bool isDodgeJumpActive = false;

bool isMoveAboutFacePressed = false;
bool isMoveAboutFaceActive = false;

bool isHoldDoubleClickPressed = false;
bool isToggleDoubleClickPressed = false;

UINT AddonWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	// set window handle
	hGame = hWnd;

	if (Settings::isSettingKeybind)
	{
		if (WM_KEYDOWN == uMsg || WM_SYSKEYDOWN == uMsg)
		{
			Keybind kb {};

			kb.Alt = GetKeyState(VK_MENU) & 0x8000;
			kb.Ctrl = GetKeyState(VK_CONTROL) & 0x8000;
			kb.Shift = GetKeyState(VK_SHIFT) & 0x8000;
			kb.Key = Keybinds::GetKeyStateFromLParam(lParam);

			// if shift, ctrl or alt set key to 0
			if (wParam == VK_SHIFT || wParam == VK_CONTROL || wParam == VK_MENU)
			{
				kb.Key = 0;
				if (wParam == VK_SHIFT) { kb.Shift = true; }
				if (wParam == VK_CONTROL) { kb.Ctrl = true; }
				if (wParam == VK_MENU) { kb.Alt = true; }
			}

			Settings::CurrentKeybind = kb;

			return 0;
		}
	}
	else
	{
		// check keybinds
		isDodgeJumpPressed = Keybinds::isKeyDown(Settings::DodgeJumpKeybind);
		isMoveAboutFacePressed = Keybinds::isKeyDown(Settings::MoveAboutFaceKeybind);
		isHoldDoubleClickPressed = Keybinds::isKeyDown(Settings::HoldDoubleClickKeybind);
		isToggleDoubleClickPressed = Keybinds::isKeyDown(Settings::ToggleDoubleClickKeybind);
	}

	return uMsg;
}

void AddonRender()
{
	if (!NexusLink || !MumbleLink || !MumbleIdentity || MumbleLink->Context.IsMapOpen || !NexusLink->IsGameplay) { return; }

	if (!MumbleLink->Context.IsTextboxFocused)
	{
		/***********************************************************************
		 * Dodge-Jump
		 **********************************************************************/
		if (isDodgeJumpPressed)
		{
			Keybinds::KeyDown(hGame, Settings::JumpKeybind);
			Keybinds::KeyDown(hGame, Settings::DodgeKeybind);

			isDodgeJumpActive = true;
		}
		else if (isDodgeJumpActive)
		{
			Keybinds::KeyUp(hGame, Settings::JumpKeybind);
			Keybinds::KeyUp(hGame, Settings::DodgeKeybind);

			isDodgeJumpActive = false;
		}

		/***********************************************************************
		 * Move About Face
		 **********************************************************************/
		if (isMoveAboutFacePressed)
		{
			// hold camera
			Keybinds::RMouseButtonUp(hGame);	// TODO: Does this actually help?
			Keybinds::LMouseButtonDown(hGame);

			// start moving forward
			Keybinds::KeyDown(hGame, Settings::MoveForwardKeybind);

			// turn character about face
			if (!isMoveAboutFaceActive)
			{
				Keybinds::KeyDown(hGame, Settings::AboutFaceKeybind);
				Keybinds::KeyUp(hGame, Settings::AboutFaceKeybind);
			}

			isMoveAboutFaceActive = true;
		}
		else if (isMoveAboutFaceActive)
		{
			// turn character about face
			Keybinds::RMouseButtonDown(hGame);
			Keybinds::RMouseButtonUp(hGame);

			// stop moving forward
			Keybinds::KeyUp(hGame, Settings::MoveForwardKeybind);

			// release camera
			Keybinds::LMouseButtonUp(hGame);

			isMoveAboutFaceActive = false;
		}
		
		/***********************************************************************
		 * Hold Double-Click
		 **********************************************************************/
		if (isHoldDoubleClickPressed)
		{
			Keybinds::LMouseButtonDblClk(hGame);
			Keybinds::LMouseButtonUp(hGame);
		}

		/***********************************************************************
		 * Toggle Double-Click
		 **********************************************************************/
		if (isToggleDoubleClickPressed)
		{
			Settings::ToggleDoubleClickModal();
		}
	}
}

void AddonOptions()
{
	static const ImGuiTreeNodeFlags IMGUI_COLLAPSING_DEFAULT_OPEN = 0x20;
	static const ImGuiTableFlags IMGUI_TABLE_BORDERS_INNER_H = 0x80;

	if (ImGui::CollapsingHeader("Movement", IMGUI_COLLAPSING_DEFAULT_OPEN))
	{
		ImGui::BeginTable("Movement", 3, IMGUI_TABLE_BORDERS_INNER_H);
		Settings::KeybindButton("Move Forward", Settings::MoveForwardKeybind, "This should match your in-game keybind for \'Move Forward.\'\n");
		Settings::KeybindButton("About Face", Settings::AboutFaceKeybind, "This should match your in-game keybind for \'About Face.\'\n");
		Settings::KeybindButton("Move About Face", Settings::MoveAboutFaceKeybind, "Hold to move your character backwards without rotating the camera.\n");
		Settings::KeybindButton("Dodge", Settings::DodgeKeybind, "This should match your in-game keybind for \'Dodge.\'\n");
		Settings::KeybindButton("Jump", Settings::JumpKeybind, "This should match your in-game keybind for \'Jump.\'\n");
		Settings::KeybindButton("Dodge-Jump", Settings::DodgeJumpKeybind, "Perform the dodge and jump actions simultaneously.\n");
		ImGui::EndTable();
	}

	if (ImGui::CollapsingHeader("Camera", IMGUI_COLLAPSING_DEFAULT_OPEN))
	{
		ImGui::BeginTable("Camera", 3, IMGUI_TABLE_BORDERS_INNER_H);
		ImGui::TableNextRow(); ImGui::TableNextColumn(); ImGui::Text("Zoom In"); ImGui::TableNextColumn(); ImGui::TableNextColumn();
		ImGui::TableNextRow(); ImGui::TableNextColumn(); ImGui::Text("Zoom Out"); ImGui::TableNextColumn(); ImGui::TableNextColumn();
		ImGui::TableNextRow(); ImGui::TableNextColumn(); ImGui::Text("Auto-Adjust Zoom"); ImGui::TableNextColumn(); ImGui::TableNextColumn();
		ImGui::EndTable();
	}

	if (ImGui::CollapsingHeader("Utilities", IMGUI_COLLAPSING_DEFAULT_OPEN))
	{
		ImGui::BeginTable("Utilities", 3, IMGUI_TABLE_BORDERS_INNER_H);
		Settings::KeybindButton("Hold Double-Click", Settings::HoldDoubleClickKeybind, "Hold button to repeatedly double-click at your cursor's current position.\n");
		Settings::KeybindButton("Toggle Double-Click", Settings::ToggleDoubleClickKeybind, "Set a timer to recurringly double-click at your cursor's current position.\n");
		ImGui::EndTable();
	}
}
