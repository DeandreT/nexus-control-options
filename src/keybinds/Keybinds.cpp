#include <algorithm>
#include <map>

#include "Keybinds.h"

bool operator==(const Keybind& lhs, const Keybind& rhs)
{
	return lhs.Key == rhs.Key &&
		lhs.Alt == rhs.Alt &&
		lhs.Ctrl == rhs.Ctrl &&
		lhs.Shift == rhs.Shift;
}

bool operator!=(const Keybind& lhs, const Keybind& rhs)
{
	return !(lhs == rhs);
}

struct KeystrokeMessageFlags
{
	unsigned RepeatCount : 16;
	unsigned ScanCode : 8;
	unsigned ExtendedFlag : 1;
	unsigned Reserved : 4;
	unsigned ContextCode : 1;
	unsigned PreviousKeyState : 1;
	unsigned TransitionState : 1;
};

LPARAM GetLParam(uint32_t key, bool down);
KeystrokeMessageFlags& LParamToKMF(LPARAM& lParam);
LPARAM& KMFToLParam(KeystrokeMessageFlags& kmf);
unsigned short GetScanCode(KeystrokeMessageFlags& kmf);
void InitialiseScanCodeLookupTable();
const char* ConvertToUTF8(const char* multibyteStr);

static std::map<unsigned short, std::string> ScanCodeLookupTable;
static bool isScanCodeLookupTableInitialised = false;

namespace Keybinds
{
	/***************************************************************************
	 * Get key positions
	 **************************************************************************/

	bool isKeyDown(Keybind& keybind)
	{
		if (keybind == Keybind{}) { return false; }

		bool alt = keybind.Alt ? GetKeyState(VK_MENU) & 0x8000 : true;
		bool shift = keybind.Shift ? GetKeyState(VK_SHIFT) & 0x8000 : true;
		bool ctrl = keybind.Ctrl ? GetKeyState(VK_CONTROL) & 0x8000 : true;
		bool key = GetKeyState(MapVirtualKey(keybind.Key, MAPVK_VSC_TO_VK)) & 0x8000;

		return (key && alt && shift && ctrl);
	}

	/***************************************************************************
	 * Set key positions
	 **************************************************************************/

	void KeyDown(HWND hWnd, Keybind& keybind)
	{
		if (keybind == Keybind{}) { return; }

		if (keybind.Alt) {
			PostMessage(hWnd, WM_SYSKEYDOWN, VK_MENU, GetLParam(VK_MENU, false));
			Sleep(5);
		}
		if (keybind.Shift) {
			PostMessage(hWnd, WM_KEYDOWN, VK_SHIFT, GetLParam(VK_SHIFT, false));
			Sleep(5);
		}
		if (keybind.Ctrl) {
			PostMessage(hWnd, WM_KEYDOWN, VK_CONTROL, GetLParam(VK_CONTROL, false));
			Sleep(5);
		}
		if (keybind.Key) {
			auto vk = MapVirtualKey(keybind.Key, MAPVK_VSC_TO_VK);
			PostMessage(hWnd, WM_KEYDOWN, vk, GetLParam(vk, false));
			Sleep(5);
		}
	}

	void KeyUp(HWND hWnd, Keybind& keybind)
	{
		if (keybind == Keybind{}) { return; }

		if (keybind.Key) {
			auto vk = MapVirtualKey(keybind.Key, MAPVK_VSC_TO_VK);
			PostMessage(hWnd, WM_KEYUP, vk, GetLParam(vk, true));
			Sleep(5);
		}
		if (keybind.Ctrl) {
			PostMessage(hWnd, WM_KEYUP, VK_CONTROL, GetLParam(VK_CONTROL, true));
			Sleep(5);
		}
		if (keybind.Shift) {
			PostMessage(hWnd, WM_KEYUP, VK_SHIFT, GetLParam(VK_SHIFT, true));
			Sleep(5);
		}
		if (keybind.Alt) {
			PostMessage(hWnd, WM_SYSKEYUP, VK_MENU, GetLParam(VK_MENU, true));
			Sleep(5);
		}
	}

	void LMouseButtonDown(HWND hWnd)
	{
		POINT CursorPos;
		if (GetCursorPos(&CursorPos))
		{
			if (ScreenToClient(hWnd, &CursorPos))
			{
				PostMessage(hWnd, WM_LBUTTONDOWN, MK_LBUTTON, MAKELPARAM(CursorPos.x, CursorPos.y));
			Sleep(5);
			}
		}
	}

	void LMouseButtonDblClk(HWND hWnd)
	{
		POINT CursorPos;
		if (GetCursorPos(&CursorPos))
		{
			if (ScreenToClient(hWnd, &CursorPos))
			{
				PostMessage(hWnd, WM_LBUTTONDBLCLK, MK_LBUTTON, MAKELPARAM(CursorPos.x, CursorPos.y));
			Sleep(5);
			}
		}
	}

	void LMouseButtonUp(HWND hWnd)
	{
		POINT CursorPos;
		if (GetCursorPos(&CursorPos))
		{
			if (ScreenToClient(hWnd, &CursorPos))
			{
				PostMessage(hWnd, WM_LBUTTONUP, MK_LBUTTON, MAKELPARAM(CursorPos.x, CursorPos.y));
			Sleep(5);
			}
		}
	}

	void RMouseButtonDown(HWND hWnd)
	{
		POINT CursorPos;
		if (GetCursorPos(&CursorPos))
		{
			if (ScreenToClient(hWnd, &CursorPos))
			{
				PostMessage(hWnd, WM_RBUTTONDOWN, MK_RBUTTON, MAKELPARAM(CursorPos.x, CursorPos.y));
			Sleep(5);
			}
		}
	}

	void RMouseButtonDblClk(HWND hWnd)
	{
		POINT CursorPos;
		if (GetCursorPos(&CursorPos))
		{
			if (ScreenToClient(hWnd, &CursorPos))
			{
				PostMessage(hWnd, WM_RBUTTONDBLCLK, MK_RBUTTON, MAKELPARAM(CursorPos.x, CursorPos.y));
			Sleep(5);
			}
		}
	}

	void RMouseButtonUp(HWND hWnd)
	{
		POINT CursorPos;
		if (GetCursorPos(&CursorPos))
		{
			if (ScreenToClient(hWnd, &CursorPos))
			{
				PostMessage(hWnd, WM_RBUTTONUP, MK_RBUTTON, MAKELPARAM(CursorPos.x, CursorPos.y));
			Sleep(5);
			}
		}
	}

	void ScrollWheel(HWND hWnd, bool scrollDown, float wheelRotations)
	{
		POINT CursorPos;
		if (GetCursorPos(&CursorPos))
		{
			if (ScreenToClient(hWnd, &CursorPos))
		{
			WPARAM wParam = MAKEWPARAM((scrollDown ? -1 : 1), wheelRotations, 0);
			LPARAM lParam = MAKELPARAM(CursorPos.x, CursorPos.y);

			PostMessage(hWnd, WM_MOUSEWHEEL, wParam, lParam);
			}
		}
	}

	/***************************************************************************
	 * Utilities
	 **************************************************************************/

	unsigned short GetKeyStateFromLParam(LPARAM lParam)
	{
		return GetScanCode(LParamToKMF(lParam));
	}

	std::string KeybindToString(Keybind& keybind)
	{
		if (keybind == Keybind{}) { return "(null)"; }

		char* buff = new char[100];
		std::string str;

		if (keybind.Alt) {
			GetKeyNameTextA(MapVirtualKeyA(VK_MENU, MAPVK_VK_TO_VSC) << 16, buff, 100);
			str.append(buff).append(" + ");
		}
		if (keybind.Ctrl) {
			GetKeyNameTextA(MapVirtualKeyA(VK_CONTROL, MAPVK_VK_TO_VSC) << 16, buff, 100);
			str.append(buff).append(" + ");
		}
		if (keybind.Shift) {
			GetKeyNameTextA(MapVirtualKeyA(VK_SHIFT, MAPVK_VK_TO_VSC) << 16, buff, 100);
			str.append(buff).append(" + ");
		}

		auto vk = MapVirtualKeyA(keybind.Key, MAPVK_VSC_TO_VK);
		if (vk >= 65 && vk <= 90 || vk >= 48 && vk <= 57)
		{
			// keys in range [A, Z] and [0, 9]
			GetKeyNameTextA(keybind.Key << 16, buff, 100);
			str.append(buff);
		}
		else
		{
			if (!isScanCodeLookupTableInitialised)
			{
				InitialiseScanCodeLookupTable();
			}

			auto it = ScanCodeLookupTable.find(keybind.Key);
			if (it != ScanCodeLookupTable.end())
			{
				str.append(it->second);
			}
		}

		delete[] buff;

		std::transform(str.begin(), str.end(), str.begin(), ::toupper);

		// Convert Multibyte encoding to UFT-8 bytes
		const char* multibyte_pointer = str.c_str();
		const char* utf8_bytes = ConvertToUTF8(multibyte_pointer);

		return std::string(utf8_bytes);
	}
} // namespace Keybinds

LPARAM GetLParam(uint32_t key, bool isKeyRelease)
{
	LPARAM lParam = 0;
	auto scanCode = MapVirtualKeyA(key, MAPVK_VK_TO_VSC_EX);
	bool isExtendedKey = false;

	lParam |= (isKeyRelease ? 1 : 0) << 31; // Transition state
	lParam |= (isKeyRelease ? 1 : 0) << 30; // Previous key state
	lParam |= 0 << 29; // Context code
	lParam |= 0 << 25; // Reserved
	lParam |= (isExtendedKey ? 1 : 0) << 24; // Extended key
	lParam |= scanCode << 16; // Scan code
	lParam |= 1; // Repeat count

	return lParam;
}

KeystrokeMessageFlags& LParamToKMF(LPARAM& lParam)
{
	return *(KeystrokeMessageFlags*)&lParam;
}

LPARAM& KMFToLParam(KeystrokeMessageFlags& kmf)
{
	return *(LPARAM*)&kmf;
}

unsigned short GetScanCode(KeystrokeMessageFlags& kmf)
{
	unsigned short scanCode = kmf.ScanCode;

	if (kmf.ExtendedFlag)
	{
		scanCode |= 0xE000;
	}

	return scanCode;
}

void InitialiseScanCodeLookupTable()
{
	for (long long i = 0; i < 255; i++)
	{
		struct KeystrokeMessageFlags key {};
		key.ScanCode = i;
		char* buff = new char[64];
		std::string str;
		GetKeyNameTextA(static_cast<LONG>(KMFToLParam(key)), buff, 64);
		str.append(buff);

		ScanCodeLookupTable[GetScanCode(key)] = str;

		key.ExtendedFlag = 1;
		buff = new char[64];
		str = "";
		GetKeyNameTextA(static_cast<LONG>(KMFToLParam(key)), buff, 64);
		str.append(buff);

		ScanCodeLookupTable[GetScanCode(key)] = str;

		delete[] buff;
	}

	isScanCodeLookupTableInitialised = true;
}

const char* ConvertToUTF8(const char* multibyteStr)
{
	char* utf8Str = nullptr;

	int wideCharCount = MultiByteToWideChar(CP_ACP, 0, multibyteStr, -1, NULL, 0);
	if (wideCharCount > 0)
	{
		wchar_t* wideCharBuff = new wchar_t[wideCharCount];
		MultiByteToWideChar(CP_ACP, 0, multibyteStr, -1, wideCharBuff, wideCharCount);

		int utf8Count = WideCharToMultiByte(CP_UTF8, 0, wideCharBuff, -1, NULL, 0, NULL, NULL);
		if (utf8Count > 0)
		{
			utf8Str = new char[utf8Count];
			WideCharToMultiByte(CP_UTF8, 0, wideCharBuff, -1, utf8Str, utf8Count, NULL, NULL);
		}

		delete[] wideCharBuff;
	}

	return utf8Str;
}
