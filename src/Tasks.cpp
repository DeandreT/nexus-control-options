#include <chrono>

#include "keybinds/Keybinds.h"

#include "Tasks.h"
#include "Settings.h"

namespace Tasks
{
	/***************************************************************************
	 * Dodge-Jump
	 **************************************************************************/

	bool isDodgeJumpDown = false;
	bool isDodgeJumpActive = false;

	void DodgeJump(HWND hWnd)
	{
		if (isDodgeJumpDown)
		{
			Keybinds::KeyDown(hWnd, Settings::JumpKeybind);
			Keybinds::KeyDown(hWnd, Settings::DodgeKeybind);

			isDodgeJumpActive = true;
		}
		else if (isDodgeJumpActive)
		{
			Keybinds::KeyUp(hWnd, Settings::JumpKeybind);
			Keybinds::KeyUp(hWnd, Settings::DodgeKeybind);

			isDodgeJumpActive = false;
		}
	}


	/***************************************************************************
	 * Move About Face
	 **************************************************************************/

	bool isMoveAboutFaceDown = false;
	bool isMoveAboutFaceActive = false;

	void MoveAboutFace(HWND hWnd)
	{
		if (isMoveAboutFaceDown)
		{
			// hold camera
			Keybinds::RMouseButtonUp(hWnd);	// TODO: Does this actually help?
			Keybinds::LMouseButtonDown(hWnd);

			// start moving forward
			Keybinds::KeyDown(hWnd, Settings::MoveForwardKeybind);

			// turn character about face
			if (!isMoveAboutFaceActive)
			{
				Keybinds::KeyDown(hWnd, Settings::AboutFaceKeybind);
				Keybinds::KeyUp(hWnd, Settings::AboutFaceKeybind);
			}

			isMoveAboutFaceActive = true;
		}
		else if (isMoveAboutFaceActive)
		{
			// turn character about face
			Keybinds::RMouseButtonDown(hWnd);
			Keybinds::RMouseButtonUp(hWnd);

			// stop moving forward
			Keybinds::KeyUp(hWnd, Settings::MoveForwardKeybind);

			// release camera
			Keybinds::LMouseButtonUp(hWnd);

			isMoveAboutFaceActive = false;
		}
	}


	/***************************************************************************
	 * Hold Double-Click
	 **************************************************************************/

	bool isHoldDoubleClickDown = false;

	void HoldDoubleClick(HWND hWnd)
	{
		if (isHoldDoubleClickDown)
		{
			Keybinds::LMouseButtonDblClk(hWnd);
			Keybinds::LMouseButtonUp(hWnd);
		}
	}


	/***************************************************************************
	 * Set Double-Click
	 **************************************************************************/

	bool isSetDoubleClickDown = false;
	bool isSetDoubleClickReleased = true;
	auto doubleClickTimeout = std::chrono::system_clock::now().time_since_epoch();

	void SetDoubleClick(HWND hWnd)
	{
		if (isSetDoubleClickDown || Settings::isSettingDoubleClick)
		{
			if (!Settings::isDoubleClickActive)
			{
				// activate double-click 
				if (isSetDoubleClickReleased)
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
				doubleClickTimeout = std::chrono::system_clock::now().time_since_epoch() + std::chrono::milliseconds(1000);
				isSetDoubleClickReleased = false;
			}

		}
		else if (Settings::isDoubleClickActive)
		{
			if (std::chrono::system_clock::now().time_since_epoch() > doubleClickTimeout)
			{
				// get current cursor position
				POINT CursorPos;
				GetCursorPos(&CursorPos);

				// set cursor position and double-click
				SetCursorPos(Settings::doubleClickPosX, Settings::doubleClickPosY);
				Keybinds::LMouseButtonDblClk(hWnd);
				Keybinds::LMouseButtonUp(hWnd);

				// restore previous cursor position
				SetCursorPos(CursorPos.x, CursorPos.y);

				// set timeout interval
				auto doubleClickIntervalMs = std::chrono::milliseconds(static_cast<int>(Settings::doubleClickInterval * 1000));
				doubleClickTimeout = std::chrono::system_clock::now().time_since_epoch() + doubleClickIntervalMs;
			}
		}
		else
		{
			isSetDoubleClickReleased = true;
		}
	}
} // namespace Tasks
