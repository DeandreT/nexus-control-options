#ifndef TASKS_H
#define TASKS_H

#include "Shared.h"

namespace Tasks {
	extern Texture* DoubleClickIndicator;

	extern bool isDodgeJumpDown;
	extern bool isMoveAboutFaceDown;
	extern bool isHoldDoubleClickDown;
	extern bool isSetDoubleClickDown;

	void DodgeJump(HWND hWnd);
	void MoveAboutFace(HWND hWnd);
	void HoldDoubleClick(HWND hWnd);
	void SetDoubleClick(HWND hWnd);
	void AutoAdjustZoom(HWND hWnd);
} // namespace Tasks

#endif /* TASKS_H */