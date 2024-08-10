#ifndef TASKS_H
#define TASKS_H

#include "Shared.h"

namespace Tasks {
	extern Texture* TexDblClk_0;
	extern Texture* TexDblClk_1;
	extern Texture* TexDblClk_2;
	extern Texture* TexDblClk_3;
	extern Texture* TexDblClk_4;

	void DodgeJump(const char* aIdentifier, bool aIsRelease);
	void MoveAboutFace(const char* aIdentifier, bool aIsRelease);
	void HoldDoubleClick(const char* aIdentifier, bool aIsRelease);
	void ToggleDoubleClick(const char* aIdentifier, bool aIsRelease);
	void PerformDoubleClick();
	void ManualAdjustZoom(const char* aIdentifier, bool aIsRelease);
	void AutoAdjustZoom();
} // namespace Tasks

#endif /* TASKS_H */