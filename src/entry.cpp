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

	NexusLink = (NexusLinkData*)APIDefs->DataLink.Get("DL_NEXUS_LINK");
	MumbleLink = (Mumble::Data*)APIDefs->DataLink.Get("DL_MUMBLE_LINK");

	APIDefs->Events.Subscribe("EV_MUMBLE_IDENTITY_UPDATED", OnMumbleIdentityUpdated);

	APIDefs->Renderer.Register(ERenderType_Render, AddonRender);
	APIDefs->Renderer.Register(ERenderType_OptionsRender, AddonOptions);

	APIDefs->InputBinds.RegisterWithString("KB_CO_DODGE_JUMP", Tasks::DodgeJump, "(null)");
	APIDefs->InputBinds.RegisterWithString("KB_CO_MOVE_ABOUT_FACE", Tasks::MoveAboutFace, "(null)");
	APIDefs->InputBinds.RegisterWithString("KB_CO_HOLD_DOUBLE_CLICK", Tasks::HoldDoubleClick, "(null)");
	APIDefs->InputBinds.RegisterWithString("KB_CO_SET_DOUBLE_CLICK", Tasks::SetDoubleClick, "(null)");

	APIDefs->WndProc.Register(AddonWndProc);

	std::filesystem::create_directory(APIDefs->Paths.GetAddonDirectory("ControlOptions"));
	Settings::Load(APIDefs->Paths.GetAddonDirectory("ControlOptions/settings.json"));
}

void AddonUnload()
{
	APIDefs->WndProc.Deregister(AddonWndProc);

	APIDefs->InputBinds.Deregister("KB_CO_DODGE_JUMP");
	APIDefs->InputBinds.Deregister("KB_CO_MOVE_ABOUT_FACE");
	APIDefs->InputBinds.Deregister("KB_CO_HOLD_DOUBLE_CLICK");
	APIDefs->InputBinds.Deregister("KB_CO_SET_DOUBLE_CLICK");

	APIDefs->Renderer.Deregister(AddonOptions);
	APIDefs->Renderer.Deregister(AddonRender);

	APIDefs->Events.Unsubscribe("EV_MUMBLE_IDENTITY_UPDATED", OnMumbleIdentityUpdated);

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

	return uMsg;
}

void AddonRender()
{
	if (!NexusLink || !MumbleLink || !MumbleIdentity) { /* wait for AddonLoad */ return; }
	if (MumbleLink->Context.IsMapOpen || !NexusLink->IsGameplay) { /* don't run macros */ return; }

	std::future<void> taskAutoAdjustZoom = std::async(std::launch::async, Tasks::AutoAdjustZoom);
	std::future<void> taskPerformDoubleClick = std::async(std::launch::async, Tasks::PerformDoubleClick);

	taskAutoAdjustZoom.wait();
	taskPerformDoubleClick.wait();
}

void AddonOptions()
{
	if (ImGui::CollapsingHeader("Movement", ImGuiTreeNodeFlags_DefaultOpen))
	{
		ImGui::BeginTable("Movement", 3, ImGuiTableFlags_BordersInnerH);
		ImGui::EndTable();
	}

	if (ImGui::CollapsingHeader("Camera", ImGuiTreeNodeFlags_DefaultOpen))
	{
		ImGui::BeginTable("Camera", 3, ImGuiTableFlags_BordersInnerH);
		Settings::SettingToggle("Auto-Adjust Zoom", Settings::AutoAdjustZoomEnabled, "Automatically zoom your camera out when the FoV changes.");
		ImGui::EndTable();
	}

	if (ImGui::CollapsingHeader("Utilities", ImGuiTreeNodeFlags_DefaultOpen))
	{
		ImGui::BeginTable("Utilities", 3, ImGuiTableFlags_BordersInnerH);
		ImGui::EndTable();
	}
}
