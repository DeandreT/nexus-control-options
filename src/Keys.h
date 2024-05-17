#ifndef KEYS_H
#define KEYS_H

#include <algorithm>
#include <map>
#include <string>

#include "Shared.h"

extern bool operator==(const Keybind& lhs, const Keybind& rhs);
extern bool operator!=(const Keybind& lhs, const Keybind& rhs);

namespace Keys
{
	// get key positions
	bool isKeyDown(Keybind& keybind);

	// set key positions
	void KeyDown(HWND hWnd, Keybind& keybind);
	void LMouseButtonDown(HWND hWnd);
	void RMouseButtonDown(HWND hWnd);
	void KeyUp(HWND hWnd, Keybind& keybind);
	void LMouseButtonUp(HWND hWnd);
	void RMouseButtonUp(HWND hWnd);

	// scancode lookup table
	extern std::map<unsigned short, std::string> ScancodeLookupTable;
	void GenerateScancodeLookupTable();

	// keystroke message flags
	struct KeystrokeMessageFlags
	{
		unsigned RepeatCount : 16;
		unsigned ScanCode : 8;
		unsigned ExtendedFlag : 1;
		unsigned Reserved : 4;
		unsigned ContextCode : 1;
		unsigned PreviousKeyState : 1;
		unsigned TransitionState : 1;

		unsigned short GetScanCode()
		{
			unsigned short ret = ScanCode;

			if (ExtendedFlag)
			{
				ret |= 0xE000;
			}

			return ret;
		}
	};

	// utilities
	std::string KeybindToString(Keybind& keybind);
	const char* ConvertToUTF8(const char* multibyteStr);
	KeystrokeMessageFlags& LParamToKMF(LPARAM& lParam);
	LPARAM& KMFToLParam(KeystrokeMessageFlags& kmf);
	LPARAM GetLParam(uint32_t key, bool down);
} // namespace Keys

#endif /* KEYS_H */
