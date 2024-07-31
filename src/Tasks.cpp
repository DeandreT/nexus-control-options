#include <chrono>
#include <iostream>
#include <sstream>
#include <string>

#include "keybinds/Keybinds.h"

#include "Tasks.h"
#include "Settings.h"
#include "resource.h"

namespace Tasks
{
	// initialise in AddonLoad
	Texture* DoubleClickIndicator = nullptr;

	static bool isTimeoutElapsed(std::chrono::system_clock::duration timeout)
	{
		return (timeout < std::chrono::system_clock::now().time_since_epoch());
	}

	void DodgeJump(const char* aIdentifier, bool aIsRelease)
	{
		if (MumbleLink->Context.IsTextboxFocused || !MumbleLink->Context.IsGameFocused) { /* don't run macros */ return; }

		if (strcmp(aIdentifier, "KB_CO_DODGE_JUMP") == 0 && !aIsRelease)
		{
			if (Mumble::EMountIndex::None == MumbleLink->Context.MountIndex)
			{
				APIDefs->GameBinds.InvokeAsync(EGameBinds_MoveJump, 0);
				APIDefs->GameBinds.InvokeAsync(EGameBinds_MoveDodge, 0);
			}
		}
	}

	void MoveAboutFace(const char* aIdentifier, bool aIsRelease)
	{
		static bool isActive = false;

		if (MumbleLink->Context.IsTextboxFocused || !MumbleLink->Context.IsGameFocused) { /* don't run macros */ return; }

		if (strcmp(aIdentifier, "KB_CO_MOVE_ABOUT_FACE") == 0)
		{
			if (!aIsRelease)
			{
				if (!isActive)
				{
					// hold camera
					Keybinds::LMouseButtonDown(hClient);

					// start moving forward
					APIDefs->GameBinds.PressAsync(EGameBinds_MoveForward);

					// turn character about face
					APIDefs->GameBinds.InvokeAsync(EGameBinds_MoveAboutFace, 0);

					isActive = true;
				}
			}
			else if (isActive)
			{
				// turn character about face
				Keybinds::RMouseButtonDown(hClient);
				Keybinds::RMouseButtonUp(hClient);

				// stop moving forward
				APIDefs->GameBinds.ReleaseAsync(EGameBinds_MoveForward);

				// release camera
				Keybinds::LMouseButtonUp(hClient);

				isActive = false;
			}
		}
	}

	void HoldDoubleClick(const char* aIdentifier, bool aIsRelease)
	{
		if (strcmp(aIdentifier, "KB_CO_HOLD_DOUBLE_CLICK") == 0)
		{
			if (!aIsRelease)
			{
				Settings::isDoubleClickActive = true;
				Settings::isDoubleClickPosFixed = false;
				Settings::doubleClickInterval = 0.75F; /** TODO: Configurable? */
			}
			else
			{
				Settings::isDoubleClickActive = false;
			}
		}
	}

	void SetDoubleClick(const char* aIdentifier, bool aIsRelease)
	{
		if (((strcmp(aIdentifier, "KB_CO_SET_DOUBLE_CLICK") == 0) && !aIsRelease))
		{
			if (!Settings::isDoubleClickActive)
			{
				// activate double-click modal
				Settings::isSettingDoubleClick = true;
			}
			else
			{
				// deactivate double-click
				Settings::isDoubleClickActive = false;
			}
		}
	}

	void PerformDoubleClick()
	{
		static auto timeout = std::chrono::system_clock::now().time_since_epoch();

		if (Settings::isDoubleClickActive)
		{
			if (isTimeoutElapsed(timeout))
			{
				if (!Settings::isDoubleClickPosFixed)
				{
					// double-click at current cursor position
					Keybinds::LMouseButtonDblClk(hClient);
					Keybinds::LMouseButtonUp(hClient);
				}
				else
				{
					// get current cursor position
					POINT CursorPos;
					GetCursorPos(&CursorPos);

					// go to configured cursor position and double-click
					SetCursorPos(Settings::doubleClickCursorPos.x, Settings::doubleClickCursorPos.y);
					Keybinds::LMouseButtonDblClk(hClient);
					Keybinds::LMouseButtonUp(hClient);

					// set previous cursor position
					SetCursorPos(CursorPos.x, CursorPos.y);
				}

				// set timeout interval
				auto doubleClickIntervalMs = std::chrono::milliseconds(static_cast<int>(Settings::doubleClickInterval * 1000));
				timeout = std::chrono::system_clock::now().time_since_epoch() + doubleClickIntervalMs;
			}

			// render indicators
			if (nullptr != DoubleClickIndicator && nullptr != DoubleClickIndicator->Resource)
			{
				// get cursor position
				POINT CursorPos = { 0 };
				if (Settings::isDoubleClickPosFixed) { CursorPos = Settings::doubleClickCursorPos; } else { GetCursorPos(&CursorPos); }
				ScreenToClient(hClient, &CursorPos);

				// render double-click indicator
				if (ImGui::Begin("Double-Click", (bool *)0, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoInputs))
				{
					ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, { 0.f, 0.f });
					ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, { 0.f, 0.f });
					ImGui::SetWindowPos(ImVec2((CursorPos.x - (DoubleClickIndicator->Width / 2) - 4), (CursorPos.y - (DoubleClickIndicator->Height / 2) - 4)));
					ImGui::Image(DoubleClickIndicator->Resource, ImVec2(DoubleClickIndicator->Width, DoubleClickIndicator->Height));
					ImGui::PopStyleVar(2);
				}
				ImGui::End();
				
				// render timeout indicator
				if (Settings::isDoubleClickPosFixed)
				{
					if (ImGui::Begin("Timeout", (bool*)0, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoInputs))
					{
						ImGui::SetWindowPos(ImVec2((CursorPos.x + (DoubleClickIndicator->Width / 2)), (CursorPos.y - (DoubleClickIndicator->Height / 2))));
						auto countdown = std::chrono::duration<double>(timeout).count() - std::chrono::duration<double>(std::chrono::system_clock::now().time_since_epoch()).count();
						std::stringstream ss;
						ss << std::fixed << std::setprecision(2) << countdown << "s (press \'" << "(null)" << "\' to stop)"; /** FIXME: Replace (null) with KeybindToString */
						ImGui::Text(ss.str().c_str());
					}
					ImGui::End();
				}
			}
			else
			{
				DoubleClickIndicator = APIDefs->Textures.GetOrCreateFromResource("RES_TEX_DBLCLK", RES_TEX_DBLCLK, hSelf);
			}
		}
		else if (Settings::isSettingDoubleClick)
		{
			// render double-click modal
			std::string modalName = "Set Double-Click";
			ImGui::OpenPopup(modalName.c_str(), ImGuiPopupFlags_AnyPopupLevel);
			Settings::SetDoubleClickModal(modalName);
		}
	}

	void AutoAdjustZoom()
	{
		const auto delay = std::chrono::milliseconds(85);
		static auto nextTick = std::chrono::system_clock::now().time_since_epoch();
		static int zoomTicks = 0;
		
		static float previousFOV = 0;
		static int previousMapID = 0;

		if (Settings::AutoAdjustZoomEnabled)
		{
			if (previousFOV < MumbleIdentity->FOV)
			{
				zoomTicks += 3;
			}

			if (previousMapID != MumbleIdentity->MapID)
			{
				zoomTicks += 12;
			}

			if (zoomTicks && isTimeoutElapsed(nextTick))
			{
				APIDefs->GameBinds.InvokeAsync(EGameBinds_CameraZoomOut, 50);
				zoomTicks--;

				nextTick = std::chrono::system_clock::now().time_since_epoch() + delay;
			}

			previousFOV = MumbleIdentity->FOV;
			previousMapID = MumbleIdentity->MapID;
		}
	}

} // namespace Tasks
