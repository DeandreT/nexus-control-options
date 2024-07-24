#include <Windows.h>
#include <filesystem>
#include <future>
#include <fstream>
#include <mutex>
#include <thread>
#include <vector>

#include "nexus/Nexus.h"
#include "mumble/Mumble.h"
#include "keybinds/Keybinds.h"
#include "imgui/imgui.h"

#include "Shared.h"
#include "Settings.h"
#include "Tasks.h"
#include "Version.h"

void OnMumbleIdentityUpdated(void* aEventArgs);
void AddonLoad(AddonAPI* aApi);
void AddonUnload();
UINT AddonWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
void AddonRender();
void AddonOptions();

AddonDefinition AddonDef = {};
HMODULE hSelf = nullptr;
HWND hClient = nullptr;

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
	AddonDef.Version.Major = V_MAJOR;
	AddonDef.Version.Minor = V_MINOR;
	AddonDef.Version.Build = V_BUILD;
	AddonDef.Version.Revision = V_REVISION;
	AddonDef.Author = "Jordan";
	AddonDef.Description = "Provides additional controls and macros not available via the in-game Control Options menu.";
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

UINT AddonWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if (MumbleLink->Context.IsTextboxFocused || !MumbleLink->Context.IsGameFocused) { /* don't run macros */ return uMsg; }

	// set window handle
	hClient = hWnd;

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
		Tasks::isDodgeJumpDown = Keybinds::isKeyDown(Settings::DodgeJumpKeybind);
		Tasks::isMoveAboutFaceDown = Keybinds::isKeyDown(Settings::MoveAboutFaceKeybind);
		Tasks::isHoldDoubleClickDown = Keybinds::isKeyDown(Settings::HoldDoubleClickKeybind);
		Tasks::isSetDoubleClickDown = Keybinds::isKeyDown(Settings::SetDoubleClickKeybind);
	}

	return uMsg;
}

void AddonRender()
{
	if (!NexusLink || !MumbleLink || !MumbleIdentity) { /* wait for AddonLoad */ return; }
	if (MumbleLink->Context.IsMapOpen || !NexusLink->IsGameplay) { /* don't run macros */ return; }

	std::future<void> taskDodgeJump = std::async(std::launch::async, Tasks::DodgeJump, hClient);
	std::future<void> taskMoveAboutFace = std::async(std::launch::async, Tasks::MoveAboutFace, hClient);
	std::future<void> taskHoldDoubleClick = std::async(std::launch::async, Tasks::HoldDoubleClick, hClient);
	std::future<void> taskSetDoubleClick = std::async(std::launch::async, Tasks::SetDoubleClick, hClient);
	std::future<void> taskAutoAdjustZoom = std::async(std::launch::async, Tasks::AutoAdjustZoom, hClient);

	taskDodgeJump.wait();
	taskMoveAboutFace.wait();
	taskHoldDoubleClick.wait();
	taskSetDoubleClick.wait();
	taskAutoAdjustZoom.wait();
}

void AddonOptions()
{
	if (ImGui::CollapsingHeader("Movement", ImGuiTreeNodeFlags_DefaultOpen))
	{
		ImGui::BeginTable("Movement", 3, ImGuiTableFlags_BordersInnerH);
		Settings::KeybindButton("Move Forward", Settings::MoveForwardKeybind, "This should match your in-game keybind for \'Move Forward.\'\n");
		Settings::KeybindButton("About Face", Settings::AboutFaceKeybind, "This should match your in-game keybind for \'About Face.\'\n");
		Settings::KeybindButton("Move About Face", Settings::MoveAboutFaceKeybind, "Hold to move your character backwards without rotating the camera.\n");
		Settings::KeybindButton("Dodge", Settings::DodgeKeybind, "This should match your in-game keybind for \'Dodge.\'\n");
		Settings::KeybindButton("Jump", Settings::JumpKeybind, "This should match your in-game keybind for \'Jump.\'\n");
		Settings::KeybindButton("Dodge-Jump", Settings::DodgeJumpKeybind, "Perform the dodge and jump actions simultaneously.\n");
		ImGui::EndTable();
	}

	if (ImGui::CollapsingHeader("Camera", ImGuiTreeNodeFlags_DefaultOpen))
	{
		ImGui::BeginTable("Camera", 3, ImGuiTableFlags_BordersInnerH);
		Settings::KeybindButton("Zoom Out", Settings::ZoomOutKeybind, "This should match your in-game keybind for \'Zoom Out.\'\n");
		Settings::SettingToggle("Auto-Adjust Zoom", Settings::AutoAdjustZoomEnabled, "Automatically zoom your camera out when the FoV changes.");
		ImGui::EndTable();
	}

	if (ImGui::CollapsingHeader("Utilities", ImGuiTreeNodeFlags_DefaultOpen))
	{
		ImGui::BeginTable("Utilities", 3, ImGuiTableFlags_BordersInnerH);
		Settings::KeybindButton("Hold Double-Click", Settings::HoldDoubleClickKeybind, "Hold button to repeatedly double-click at your cursor's current position.\n");
		Settings::KeybindButton("Set Double-Click", Settings::SetDoubleClickKeybind, "Set a timer to recurringly double-click at your cursor's current\n\
position. Press this button again to end the double-click macro.");
		ImGui::EndTable();
	}
}
