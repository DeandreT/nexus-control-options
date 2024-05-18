#ifndef NEXUS_KEYBINDS_H
#define NEXUS_KEYBINDS_H

#include <string>

#include "../nexus/Nexus.h"

extern bool operator==(const Keybind& lhs, const Keybind& rhs);
extern bool operator!=(const Keybind& lhs, const Keybind& rhs);

namespace Keybinds
{
	// get key positions
	bool isKeyDown(Keybind& keybind);

	// set key positions
	void KeyDown(HWND hWnd, Keybind& keybind);
	void KeyUp(HWND hWnd, Keybind& keybind);
	void LMouseButtonDown(HWND hWnd);
	void LMouseButtonUp(HWND hWnd);
	void RMouseButtonDown(HWND hWnd);
	void RMouseButtonUp(HWND hWnd);

	// utilities
	unsigned short GetKeyStateFromLParam(LPARAM lParam);
	std::string KeybindToString(Keybind& keybind);
} // namespace Keybinds

#endif /* NEXUS_KEYBINDS_H */
