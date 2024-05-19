#ifndef TASKS_H
#define TASKS_H

#include "Shared.h"

namespace Tasks {
	void DodgeJump(HWND hWnd);
	void MoveAboutFace(HWND hWnd);
	void HoldDoubleClick(HWND hWnd);
	void SetDoubleClick(HWND hWnd);

	extern bool isDodgeJumpDown;
	extern bool isMoveAboutFaceDown;
	extern bool isHoldDoubleClickDown;
	extern bool isSetDoubleClickDown;
} // namespace Tasks

#endif /* TASKS_H */