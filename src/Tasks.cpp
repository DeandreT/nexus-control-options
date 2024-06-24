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
		
		if (Mumble::EMountIndex::None == MumbleLink->Context.MountIndex)
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
		
		if (isMoveAboutFaceDown)
		{
			if (!isActive)
			{
				// hold camera
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

	void HoldDoubleClick(HWND hWnd)
	{
		static auto timeout = std::chrono::system_clock::now().time_since_epoch();
		const auto internalCooldown = std::chrono::milliseconds(50);

		if (isHoldDoubleClickDown && !Settings::isDoubleClickActive)
		{
			if (isTimeoutElapsed(timeout))
			{
				// double-click at current position
				Keybinds::LMouseButtonDblClk(hWnd);
				Keybinds::LMouseButtonUp(hWnd);

				// set timeout interval
				timeout = std::chrono::system_clock::now().time_since_epoch() + internalCooldown;
			}

			// render double-click indicator
			if (nullptr != DoubleClickIndicator && nullptr != DoubleClickIndicator->Resource)
			{
				if (ImGui::Begin("Double-Click", (bool *)0, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoInputs))
				{
					POINT CursorPos;
					GetCursorPos(&CursorPos);
					ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, { 0.f, 0.f });
					ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, { 0.f, 0.f });
					ImGui::SetWindowPos(ImVec2((CursorPos.x - (DoubleClickIndicator->Width / 2) - 4), (CursorPos.y - (DoubleClickIndicator->Height / 2) - 4)));
					ImGui::Image(DoubleClickIndicator->Resource, ImVec2(DoubleClickIndicator->Width, DoubleClickIndicator->Height));
					ImGui::PopStyleVar(2);
				}
				ImGui::End();
			}
			else
			{
				DoubleClickIndicator = APIDefs->GetTextureOrCreateFromResource("RES_TEX_DBLCLK", RES_TEX_DBLCLK, hSelf);
			}
		}
	}

	void SetDoubleClick(HWND hWnd)
	{
		static auto timeout = std::chrono::system_clock::now().time_since_epoch();
		const auto internalCooldown = std::chrono::milliseconds(1000);

		if ((isSetDoubleClickDown || Settings::isSettingDoubleClick) && !isHoldDoubleClickDown)
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

			// render double-click indicator
			if (nullptr != DoubleClickIndicator && nullptr != DoubleClickIndicator->Resource)
			{
				if (ImGui::Begin("Double-Click", (bool *)0, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoInputs))
				{
					ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, { 0.f, 0.f });
					ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, { 0.f, 0.f });
					ImGui::SetWindowPos(ImVec2((Settings::doubleClickPosX - (DoubleClickIndicator->Width / 2) - 4), (Settings::doubleClickPosY - (DoubleClickIndicator->Height / 2) - 4)));
					ImGui::Image(DoubleClickIndicator->Resource, ImVec2(DoubleClickIndicator->Width, DoubleClickIndicator->Height));
					ImGui::PopStyleVar(2);
				}
				ImGui::End();

				if (ImGui::Begin("Timeout", (bool*)0, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoInputs))
				{
					ImGui::SetWindowPos(ImVec2((Settings::doubleClickPosX + (DoubleClickIndicator->Width / 2)), (Settings::doubleClickPosY - (DoubleClickIndicator->Height / 2))));
					auto countdown = std::chrono::duration<double>(timeout).count() - std::chrono::duration<double>(std::chrono::system_clock::now().time_since_epoch()).count();
					std::stringstream ss;
					ss << std::fixed << std::setprecision(2) << countdown << "s (press \'" << Keybinds::KeybindToString(Settings::SetDoubleClickKeybind) << "\' to stop)";
					ImGui::Text(ss.str().c_str());
				}
				ImGui::End();
			}
			else
			{
				DoubleClickIndicator = APIDefs->GetTextureOrCreateFromResource("RES_TEX_DBLCLK", RES_TEX_DBLCLK, hSelf);
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
