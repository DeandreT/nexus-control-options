#include <Windows.h>
#include <filesystem>
#include <fstream>
#include <mutex>
#include <vector>

#include "nexus/Nexus.h"
#include "mumble/Mumble.h"
#include "imgui/imgui.h"

#include "Shared.h"
#include "Settings.h"
#include "Keys.h"

void OnMumbleIdentityUpdated(void* aEventArgs);
void AddonLoad(AddonAPI* aApi);
void AddonUnload();
UINT AddonWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
void AddonRender();
void AddonOptions();

AddonDefinition AddonDef = {};
HWND Game;
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
	AddonDef.Version.Build = 17;
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

	// generate scancode lookup table for keybindings
	Keys::GenerateScancodeLookupTable();

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
bool isHoldDoubleClickActive = false;

UINT AddonWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	// set window handle
	Game = hWnd;

	if (WM_KEYDOWN == uMsg || WM_SYSKEYDOWN == uMsg)
	{
		Keybind kb{};
		kb.Alt = GetKeyState(VK_MENU) & 0x8000;
		kb.Ctrl = GetKeyState(VK_CONTROL) & 0x8000;
		kb.Shift = GetKeyState(VK_SHIFT) & 0x8000;
		kb.Key = Keys::LParamToKMF(lParam).GetScanCode();

		// if shift, ctrl or alt set key to 0
		if (wParam == VK_SHIFT || wParam == VK_CONTROL || wParam == VK_MENU)
		{
			kb.Key = 0;
			if (wParam == VK_SHIFT) { kb.Shift = true; }
			if (wParam == VK_CONTROL) { kb.Ctrl = true; }
			if (wParam == VK_MENU) { kb.Alt = true; }
		}

		if (Settings::isSettingKeybind)
		{
			Settings::CurrentKeybind = kb;
		}
		else
		{
			isDodgeJumpPressed = Keys::isKeyDown(Settings::DodgeJumpKeybind);
			isMoveAboutFacePressed = Keys::isKeyDown(Settings::MoveAboutFaceKeybind);
			isHoldDoubleClickPressed = Keys::isKeyDown(Settings::HoldDoubleClickKeybind);
		}
	}
	else if (WM_KEYUP == uMsg || WM_SYSKEYUP == uMsg)
	{
		Keybind kb{};
		kb.Alt = GetKeyState(VK_MENU) & 0x8000;
		kb.Ctrl = GetKeyState(VK_CONTROL) & 0x8000;
		kb.Shift = GetKeyState(VK_SHIFT) & 0x8000;
		kb.Key = Keys::LParamToKMF(lParam).GetScanCode();

		// if shift, ctrl or alt set key to 0
		if (wParam == VK_SHIFT || wParam == VK_CONTROL || wParam == VK_MENU)
		{
			kb.Key = 0;
			if (wParam == VK_SHIFT) { kb.Shift = true; }
			if (wParam == VK_CONTROL) { kb.Ctrl = true; }
			if (wParam == VK_MENU) { kb.Alt = true; }
		}
	}

	if (Settings::isSettingKeybind && (uMsg == WM_SYSKEYDOWN || uMsg == WM_KEYDOWN || uMsg == WM_SYSKEYUP || uMsg == WM_KEYUP))
	{
		return 0;
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
			Keys::KeyDown(Game, Settings::DodgeKeybind);
			Keys::KeyDown(Game, Settings::JumpKeybind);

			isDodgeJumpActive = true;
		}
		else if (isDodgeJumpActive)
		{
			Keys::KeyUp(Game, Settings::JumpKeybind);
			Keys::KeyUp(Game, Settings::DodgeKeybind);

			isDodgeJumpActive = false;
		}

		/***********************************************************************
		 * Move About Face
		 **********************************************************************/
		if (isMoveAboutFacePressed)
		{
			// hold camera
			Keys::RMouseButtonUp(Game);	// TODO: Does this actually help?
			Keys::LMouseButtonDown(Game);

			// start moving forward
			Keys::KeyDown(Game, Settings::MoveForwardKeybind);

			// turn character about face
			if (!isMoveAboutFaceActive)
			{
				Keys::KeyDown(Game, Settings::AboutFaceKeybind);
				Keys::KeyUp(Game, Settings::AboutFaceKeybind);
			}

			isMoveAboutFaceActive = true;
		}
		else if (isMoveAboutFaceActive)
		{
			// turn character about face
			Keys::RMouseButtonDown(Game);
			Keys::RMouseButtonUp(Game);
			
			// stop moving forward
			Keys::KeyUp(Game, Settings::MoveForwardKeybind);
			
			// release camera
			Keys::LMouseButtonUp(Game);

			isMoveAboutFaceActive = false;
		}

		/***********************************************************************
		 * Hold Double Click
		 **********************************************************************/
		if (isHoldDoubleClickPressed)
		{
			Keys::LMouseButtonDown(Game);
			Keys::LMouseButtonUp(Game);
			Keys::LMouseButtonDown(Game);
			Keys::LMouseButtonUp(Game);

			//isHoldDoubleClickActive = true;
		}
		/*else if (isHoldDoubleClickActive)
		{
			isHoldDoubleClickActive = false;
		}*/

	}
}

void AddonOptions()
{
	ImGui::Text("Control Options (In-Game Mappings)");

	Settings::KeybindButton("Move Forward", Settings::MoveForwardKeybind, "This should match your in-game keybind for \'Move Forward.\'\n");
	Settings::KeybindButton("Dodge", Settings::DodgeKeybind, "This should match your in-game keybind for \'Dodge.\'\n");
	Settings::KeybindButton("Jump", Settings::JumpKeybind, "This should match your in-game keybind for \'Jump.\'\n");
	Settings::KeybindButton("About Face", Settings::AboutFaceKeybind, "This should match your in-game keybind for \'About Face.\'\n");

	ImGui::Text("Extended Control Options");

	Settings::KeybindButton("Dodge-Jump", Settings::DodgeJumpKeybind, "Perform the dodge and jump actions simultaneously.\n");
	Settings::KeybindButton("Move About Face", Settings::MoveAboutFaceKeybind, "Hold to move your character backwards without rotating the camera.\n");

	ImGui::Text("Macros & Utilities");
	Settings::KeybindButton("Hold Double Click", Settings::HoldDoubleClickKeybind, "Hold to repeatedly double-click at your cursor's current position.\n");
}
