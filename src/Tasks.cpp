#include <chrono>

#include "keybinds/Keybinds.h"

#include "Tasks.h"
#include "Settings.h"

namespace Tasks
{
	bool isDodgeJumpDown = false;
	bool isMoveAboutFaceDown = false;
	bool isHoldDoubleClickDown = false;
	bool isSetDoubleClickDown = false;

	static bool isTimeoutElapsed(std::chrono::system_clock::duration timeout)
	{
		return (std::chrono::system_clock::now().time_since_epoch() > timeout);
	}

	void DodgeJump(HWND hWnd)
	{
		static bool isActive = false;

		if (!MumbleLink->Context.IsTextboxFocused)
		{
			if (isDodgeJumpDown)
			{			
				if (!isActive)
				{
					Keybinds::KeyDown(hWnd, Settings::JumpKeybind);
					Keybinds::KeyDown(hWnd, Settings::DodgeKeybind);

					isActive = true;
				}
			}
			else if (isActive)
			{
				Keybinds::KeyUp(hWnd, Settings::JumpKeybind);
				Keybinds::KeyUp(hWnd, Settings::DodgeKeybind);

				isActive = false;
			}
		}
	}

	void MoveAboutFace(HWND hWnd)
	{
		static bool isActive = false;
		
		if (!MumbleLink->Context.IsTextboxFocused)
		{
			if (isMoveAboutFaceDown)
			{
				if (!isActive)
				{
					// hold camera
					Keybinds::RMouseButtonUp(hWnd);	// TODO: Does this actually help?
					Keybinds::LMouseButtonDown(hWnd);

					// start moving forward
					Keybinds::KeyDown(hWnd, Settings::MoveForwardKeybind);

					// turn character about face
					Keybinds::KeyDown(hWnd, Settings::AboutFaceKeybind);
					Keybinds::KeyUp(hWnd, Settings::AboutFaceKeybind);

					isActive = true;
				}
			}
			else if (isActive)
			{
				// turn character about face
				Keybinds::RMouseButtonDown(hWnd);
				Keybinds::RMouseButtonUp(hWnd);

				// stop moving forward
				Keybinds::KeyUp(hWnd, Settings::MoveForwardKeybind);

				// release camera
				Keybinds::LMouseButtonUp(hWnd);

				isActive = false;
			}
		}
	}

	void HoldDoubleClick(HWND hWnd)
	{
		static auto timeout = std::chrono::system_clock::now().time_since_epoch();
		const auto internalCooldown = std::chrono::milliseconds(50);

		if (!MumbleLink->Context.IsTextboxFocused)
		{
			if (isHoldDoubleClickDown)
			{
				if (isTimeoutElapsed(timeout))
				{
					Keybinds::LMouseButtonDblClk(hWnd);
					Keybinds::LMouseButtonUp(hWnd);

					timeout = std::chrono::system_clock::now().time_since_epoch() + internalCooldown;
				}
			}
		}
	}

	void SetDoubleClick(HWND hWnd)
	{
		static auto timeout = std::chrono::system_clock::now().time_since_epoch();
		const auto internalCooldown = std::chrono::milliseconds(1000);

		if ((isSetDoubleClickDown || Settings::isSettingDoubleClick) && !MumbleLink->Context.IsTextboxFocused)
		{
			if (!Settings::isDoubleClickActive)
			{
				// activate double-click 
				if (isTimeoutElapsed(timeout))
				{
					std::string modalName = "Set Double-Click";
					ImGui::OpenPopup(modalName.c_str(), ImGuiPopupFlags_AnyPopupLevel);
					Settings::SetDoubleClickModal(modalName);
				}
			}
			else
			{
				// deactivate double-click
				Settings::isDoubleClickActive = false;
				timeout = std::chrono::system_clock::now().time_since_epoch() + internalCooldown;
			}
		}
		else if (Settings::isDoubleClickActive)
		{
			if (isTimeoutElapsed(timeout))
			{
				// get current cursor position
				POINT CursorPos;
				GetCursorPos(&CursorPos);

				// go to configured cursor position and double-click
				SetCursorPos(Settings::doubleClickPosX, Settings::doubleClickPosY);
				Keybinds::LMouseButtonDblClk(hWnd);
				Keybinds::LMouseButtonUp(hWnd);

				// restore previous cursor position
				SetCursorPos(CursorPos.x, CursorPos.y);

				// set timeout interval
				auto doubleClickIntervalMs = std::chrono::milliseconds(static_cast<int>(Settings::doubleClickInterval * 1000));
				timeout = std::chrono::system_clock::now().time_since_epoch() + doubleClickIntervalMs;
			}
		}
	}

	void AutoAdjustZoom(HWND hWnd)
	{
		static float previousFOV = 0.0F;
		static int zoomIncrements = 0;

		if (Settings::AutoAdjustZoomEnabled)
		{
			if (previousFOV < MumbleIdentity->FOV)
			{
				zoomIncrements += 3;
			}

			if (zoomIncrements)
			{
				Keybinds::ScrollWheel(hWnd, true, 0.5F);
				zoomIncrements--;
			}

			previousFOV = MumbleIdentity->FOV;
		}
	}

} // namespace Tasks
