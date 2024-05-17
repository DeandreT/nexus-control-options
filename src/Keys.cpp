#include "Keys.h"

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

namespace Keys
{
	/***************************************************************************
	 * Get key positions
	 **************************************************************************/

	bool isKeyDown(Keybind& keybind)
	{
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
		if (keybind.Alt) {
			PostMessage(hWnd, WM_SYSKEYDOWN, VK_MENU, GetLParam(VK_MENU, 1));
			Sleep(5);
		}
		if (keybind.Shift) {
			PostMessage(hWnd, WM_KEYDOWN, VK_SHIFT, GetLParam(VK_SHIFT, 1));
			Sleep(5);
		}
		if (keybind.Ctrl) {
			PostMessage(hWnd, WM_KEYDOWN, VK_CONTROL, GetLParam(VK_SHIFT, 1));
			Sleep(5);
		}
		if (keybind.Key) {
			auto vk = MapVirtualKey(keybind.Key, MAPVK_VSC_TO_VK);
			PostMessage(hWnd, WM_KEYDOWN, vk, GetLParam(vk, 1));
			Sleep(5);
		}
	}

	void LMouseButtonDown(HWND hWnd)
	{
		POINT MousePosition;
		if (GetCursorPos(&MousePosition))
		{
			PostMessage(hWnd, WM_LBUTTONDOWN, MK_LBUTTON, MAKELPARAM(MousePosition.x, MousePosition.y));
			Sleep(5);
		}
	}

	void RMouseButtonDown(HWND hWnd)
	{
		POINT MousePosition;
		if (GetCursorPos(&MousePosition))
		{
			PostMessage(hWnd, WM_RBUTTONDOWN, MK_RBUTTON, MAKELPARAM(MousePosition.x, MousePosition.y));
			Sleep(5);
		}
	}

	void KeyUp(HWND hWnd, Keybind& keybind)
	{
		if (keybind.Key) {
			auto vk = MapVirtualKey(keybind.Key, MAPVK_VSC_TO_VK);
			PostMessage(hWnd, WM_KEYUP, vk, GetLParam(vk, 0));
			Sleep(5);
		}
		if (keybind.Ctrl) {
			PostMessage(hWnd, WM_KEYUP, VK_CONTROL, GetLParam(VK_CONTROL, 0));
			Sleep(5);
		}
		if (keybind.Shift) {
			PostMessage(hWnd, WM_KEYUP, VK_SHIFT, GetLParam(VK_SHIFT, 0));
			Sleep(5);
		}
		if (keybind.Alt) {
			PostMessage(hWnd, WM_SYSKEYUP, VK_MENU, GetLParam(VK_MENU, 0));
			Sleep(5);
		}
	}

	void LMouseButtonUp(HWND hWnd)
	{
		POINT MousePosition;
		if (GetCursorPos(&MousePosition))
		{
			PostMessage(hWnd, WM_LBUTTONUP, MK_LBUTTON, MAKELPARAM(MousePosition.x, MousePosition.y));
			Sleep(5);
		}
	}

	void RMouseButtonUp(HWND hWnd)
	{
		POINT MousePosition;
		if (GetCursorPos(&MousePosition))
		{
			PostMessage(hWnd, WM_RBUTTONUP, MK_RBUTTON, MAKELPARAM(MousePosition.x, MousePosition.y));
			Sleep(5);
		}
	}

	/***************************************************************************
	 * Scancode Lookup Table
	 **************************************************************************/

	std::map<unsigned short, std::string> ScancodeLookupTable;

	void GenerateScancodeLookupTable()
	{
		for (long long i = 0; i < 255; i++)
		{
			struct KeystrokeMessageFlags key{};
			key.ScanCode = i;
			char* buff = new char[64];
			std::string str;
			GetKeyNameTextA(static_cast<LONG>(KMFToLParam(key)), buff, 64);
			str.append(buff);

			ScancodeLookupTable[key.GetScanCode()] = str;

			key.ExtendedFlag = 1;
			buff = new char[64];
			str = "";
			GetKeyNameTextA(static_cast<LONG>(KMFToLParam(key)), buff, 64);
			str.append(buff);

			ScancodeLookupTable[key.GetScanCode()] = str;

			delete[] buff;
		}
	}

	/***************************************************************************
	 * Utilities
	 **************************************************************************/

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
			// Keys in range [A, Z] and [0, 9]
			GetKeyNameTextA(keybind.Key << 16, buff, 100);
			str.append(buff);
		}
		else
		{
			auto it = ScancodeLookupTable.find(keybind.Key);
			if (it != ScancodeLookupTable.end())
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

	KeystrokeMessageFlags& LParamToKMF(LPARAM& lParam)
	{
		return *(KeystrokeMessageFlags*)&lParam;
	}

	LPARAM& KMFToLParam(KeystrokeMessageFlags& kmf)
	{
		return *(LPARAM*)&kmf;
	}

	LPARAM GetLParam(uint32_t key, bool down)
	{
		uint64_t lParam;
		lParam = down ? 0 : 1; // transition state
		lParam = lParam << 1;
		lParam += down ? 0 : 1; // previous key state
		lParam = lParam << 1;
		lParam += 0; // context code
		lParam = lParam << 1;
		lParam = lParam << 4;
		lParam = lParam << 1;
		lParam = lParam << 8;
		lParam += MapVirtualKeyA(key, MAPVK_VK_TO_VSC);
		lParam = lParam << 16;
		lParam += 1;

		return lParam;
	}
} // namespace Keys
