#ifndef TASKS_H
#define TASKS_H

#include "Shared.h"

namespace Tasks {
	extern Texture* DoubleClickIndicator;

	void DodgeJump(const char* aIdentifier, bool aIsRelease);
	void MoveAboutFace(const char* aIdentifier, bool aIsRelease);
	void HoldDoubleClick(const char* aIdentifier, bool aIsRelease);
	void SetDoubleClick(const char* aIdentifier, bool aIsRelease);
	void PerformDoubleClick();
	void AutoAdjustZoom();
} // namespace Tasks

#endif /* TASKS_H */