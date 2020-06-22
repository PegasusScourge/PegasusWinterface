/*

WinAssist

Static class that provides utility functions for getting windows API information, sending keys etc

*/

#include "WinAssist.h"
#include <iostream>

namespace pi = pinterface;
using namespace pi;

/*******************************************************************************
		namespace WinCallbacks
********************************************************************************/

BOOL CALLBACK WinCallbacks::winInfoList(HWND hwnd, LPARAM lParam) {
	const DWORD TITLE_SIZE = 1024;
	WCHAR windowTitle[TITLE_SIZE];

	GetWindowTextW(hwnd, windowTitle, TITLE_SIZE);
	int length = ::GetWindowTextLength(hwnd);

	// We don't process the window if the length of the title is 0
	if (length == 0) {
		return TRUE; // Exit the callback
	}

	std::vector<WinInfo_t>& infoList = *reinterpret_cast<std::vector<WinInfo_t>*>(lParam);

	WinInfo_t winInfo;
	winInfo.title = std::wstring(&windowTitle[0]);
	winInfo.isVisible = IsWindowVisible(hwnd);
	winInfo.tid = GetWindowThreadProcessId(hwnd, &winInfo.pid);

	infoList.push_back(winInfo);

	return TRUE;
}

/*******************************************************************************
		class KeyEvent, public
********************************************************************************/

KeyEvent::KeyEvent(WORD vKey, EventType type, bool scanCode, bool isExtended) {
	m_vKey = vKey;
	m_isExtended = isExtended;
	m_type = type;
	m_scanCode = scanCode;
}

KeyEvent::KeyEvent() {
	KeyEvent(0, KeyEvent::EventType::KEVT_NONE);
}

WORD KeyEvent::vKey() {
	return m_vKey;
}

bool KeyEvent::scanCode() {
	return m_scanCode;
}

bool KeyEvent::isExtended() {
	return m_isExtended;
}

KeyEvent::EventType KeyEvent::type() {
	return m_type;
}

/*******************************************************************************
		class MouseEvent, public
********************************************************************************/

MouseEvent::MouseEvent(EventType type, MouseKey vKey) {
	m_type = type;
	m_key = vKey;
}

MouseEvent::MouseEvent() {
	m_type = MouseEvent::EventType::MEVT_NONE;
	m_key = MouseEvent::MouseKey::MKEY_NONE;
}

void MouseEvent::setScrollDelta(DWORD d) {
	m_scrollDelta = d;
}

void MouseEvent::setMoveValues(LONG dx, LONG dy) {
	m_dx = dx;
	m_dy = dy;
}

MouseEvent::EventType MouseEvent::type() {
	return m_type;
}

MouseEvent::MouseKey MouseEvent::key() {
	return m_key;
}

LONG MouseEvent::dx() {
	return m_dx;
}

LONG MouseEvent::dy() {
	return m_dy;
}

DWORD MouseEvent::scrollDelta() {
	return m_scrollDelta;
}

/*******************************************************************************
		class WinAssist, private
********************************************************************************/
/* Private static variables */
std::vector<WinInfo_t> WinAssist::WIN_INFO(0);

/* Private static functinos */
void WinAssist::updateWinNames() {
	WIN_INFO.clear();
	EnumWindows(WinCallbacks::winInfoList, reinterpret_cast<LPARAM>(&WIN_INFO));
}

bool WinAssist::checkWinHwndValidity(HWND hwnd) {
	return IsWindow(hwnd);
}

void WinAssist::sendKey(KeyEvent key) {
	if (key.type() == KeyEvent::EventType::KEVT_NONE)
		return;

	INPUT input[2];
	ZeroMemory(&input[0], sizeof(INPUT));
	ZeroMemory(&input[1], sizeof(INPUT));

	WORD scanCode = MapVirtualKey(key.vKey(), MAPVK_VK_TO_VSC);
	DWORD flags = 0;
	if (key.scanCode())
		flags |= KEYEVENTF_SCANCODE;
	if (key.isExtended())
		flags |= KEYEVENTF_EXTENDEDKEY;

	std::cout << "Sending key: VK#=" << key.vKey() << ", scanCode=" << scanCode;
	if (key.scanCode())
		std::cout << ", mode=scancode";
	else
		std::cout << ", mode=vk";

	int index = 0;
	if (key.type() == KeyEvent::EventType::KEVT_TYPED || key.type() == KeyEvent::EventType::KEVT_PRESSED) {
		// Write the press information to input
		input[index].ki.wVk = key.vKey();
		input[index].ki.wScan = scanCode;
		input[index].ki.dwFlags = flags; //press down
		input[index].type = INPUT_KEYBOARD;
		std::cout << ", [PRESSED]";
		index++;
	}
	if (key.type() == KeyEvent::EventType::KEVT_TYPED || key.type() == KeyEvent::EventType::KEVT_RELEASED) {
		// Write the release information to input
		input[index].ki.wVk = key.vKey();
		input[index].ki.wScan = scanCode;
		input[index].ki.dwFlags = flags | KEYEVENTF_KEYUP; //release key
		input[index].type = INPUT_KEYBOARD;
		std::cout << ", [RELEASED]";
		index++;
	}

	std::cout << ", index == " << index;
	if (index == 0 || index > 2) {
		// Error
		std::cout << ": ERROR" << std::endl;
		return;
	}
	// Send
	std::cout << ": SEND" << std::endl;
	SendInput(index, input, sizeof(INPUT));
}

void WinAssist::sendMouse(MouseEvent evt) {
	if (evt.type() == MouseEvent::EventType::MEVT_NONE)
		return;

	std::vector<INPUT> inputQueue;

	std::cout << "Sending mouse event: typeIndex=" << (int)evt.type();

	/* KEY PRESSED DOWN */
	if (evt.type() == MouseEvent::EventType::MEVT_KEY_PRESSED || evt.type() == MouseEvent::EventType::MEVT_KEY_DOWN) {
		// Add the mouse key DOWN
		std::cout << ", [MOUSE_KEY_DOWN] (";
		INPUT in;
		ZeroMemory(&in, sizeof(INPUT));
		// Put the key information into the flags
		switch (evt.key()) {
		case MouseEvent::MouseKey::MKEY_LEFT:
			in.mi.dwFlags |= MOUSEEVENTF_LEFTDOWN;
			std::cout << "LEFT)";
			break;

		case MouseEvent::MouseKey::MKEY_RIGHT:
			in.mi.dwFlags |= MOUSEEVENTF_RIGHTDOWN;
			std::cout << "RIGHT)";
			break;

		case MouseEvent::MouseKey::MKEY_MID:
			in.mi.dwFlags |= MOUSEEVENTF_MIDDLEDOWN;
			std::cout << "MID)";
			break;

		default:
			std::cout << "NONE)";
			break;
		}
		// Add to the input queue
		inputQueue.push_back(in);
	}
	/* KEY RELEASED */
	if (evt.type() == MouseEvent::EventType::MEVT_KEY_PRESSED || evt.type() == MouseEvent::EventType::MEVT_KEY_UP) {
		// Add the mouse key UP
		std::cout << ", [MOUSE_KEY_UP] (";
		INPUT in;
		ZeroMemory(&in, sizeof(INPUT));
		// Put the key information into the flags
		switch (evt.key()) {
		case MouseEvent::MouseKey::MKEY_LEFT:
			in.mi.dwFlags |= MOUSEEVENTF_LEFTUP;
			std::cout << "LEFT)";
			break;

		case MouseEvent::MouseKey::MKEY_RIGHT:
			in.mi.dwFlags |= MOUSEEVENTF_RIGHTUP;
			std::cout << "RIGHT)";
			break;

		case MouseEvent::MouseKey::MKEY_MID:
			in.mi.dwFlags |= MOUSEEVENTF_MIDDLEUP;
			std::cout << "MID)";
			break;

		default:
			std::cout << "NONe)";
			break;
		}
		// Add to the input queue
		inputQueue.push_back(in);
	}
	/* MOUSE MOVE */
	if (evt.type() == MouseEvent::EventType::MEVT_MOVE) {
		// Add the relative mouse movement
		std::cout << ", [MOUSE_MOVE]";
		INPUT in;
		ZeroMemory(&in, sizeof(INPUT));
		// Put the values in
		in.mi.dx = evt.dx();
		in.mi.dy = evt.dy();
		std::cout << " dx=" << in.mi.dx << " dy=" << in.mi.dy;
		in.mi.dwFlags = MOUSEEVENTF_MOVE;
		// Add to the input queue
		inputQueue.push_back(in);
	}
	/* MOUSE ABS MOVE (Normal and Desktop) */
	if (evt.type() == MouseEvent::EventType::MEVT_MOVE_ABS || evt.type() == MouseEvent::EventType::MEVT_MOVE_DESKTOP) {
		// Add the absoltue mouse movement
		std::cout << ", [MOUSE_ABS_MOVE]";
		INPUT in;
		ZeroMemory(&in, sizeof(INPUT));
		// Put the values in
		in.mi.dwFlags |= MOUSEEVENTF_ABSOLUTE;
		if (evt.type() == MouseEvent::EventType::MEVT_MOVE_DESKTOP) {
			in.mi.dwFlags |= MOUSEEVENTF_VIRTUALDESK;
			std::cout << "[DESKTOP]";
		}
		in.mi.dx = evt.dx();
		in.mi.dy = evt.dy();
		std::cout << " dx=" << in.mi.dx << " dy=" << in.mi.dy;
		// Add to the input queue
		inputQueue.push_back(in);
	}
	/* MOUSE SCROLL */
	if (evt.type() == MouseEvent::EventType::MEVT_SCROLL) {
		// Add the scroll movement
		std::cout << ", [MOUSE_SCROLL]";
		INPUT in;
		ZeroMemory(&in, sizeof(INPUT));
		// Put the values in
		in.mi.dwFlags |= MOUSEEVENTF_WHEEL;
		in.mi.mouseData = evt.scrollDelta();
		std::cout << " delta=" << in.mi.mouseData;
	}

	if (inputQueue.size() == 0) {
		std::cout << " [NONE]" << std::endl;
		return;
	}
	// Dispatch the inputs
	SendInput(inputQueue.size(), &inputQueue[0], sizeof(INPUT));
	std::cout << " [SENT]" << std::endl;
}

//void WinAssist::sendKeyB(WinInfo_t window, WORD key) {
//	if (PostThreadMessage(window.tid, WM_KEYDOWN, key, 1) == ERROR_INVALID_THREAD_ID) {
//		std::cerr << "Send key (PostMessage) failed: invalid thread ID" << std::endl;
//		return;
//	}
//	std::cout << "Sent key (PostMessage) '" << (char)key << "'" << std::endl;
//}

bool WinAssist::connectToThread(WinInfo_t window) {
	return AttachThreadInput(GetCurrentThreadId(), window.tid, true);
}

bool WinAssist::disconnectThread(WinInfo_t window) {
	return AttachThreadInput(GetCurrentThreadId(), window.tid, false);
}

//void WinAssist::sendKeysB(WinInfo_t window, std::vector<WORD> keys) {
//	SetActiveWindow(getWindowHWND(window));
//	for (auto key : keys) {
//		sendKeyB(window, key);
//	}
//}

/*******************************************************************************
		class WinAssist, public
********************************************************************************/
/* Public static functions */
std::vector<WinInfo_t> WinAssist::getWindowList() {
	updateWinNames();
	return WIN_INFO;
}

std::vector<WinInfo_t> WinAssist::getVisibleWindowList() {
	updateWinNames();
	std::vector<WinInfo_t> windows;
	for (auto w : WIN_INFO) {
		if (w.isVisible) {
			windows.push_back(w);
		}
	}
	return windows;
}

void WinAssist::sendKeys(WinInfo_t window, std::vector<KeyEvent> keys) {
	// Attempt to connect to the window
	if (!connectToThread(window)) {
		std::wcerr << "Failed to send keys: unable to connect to thread of window with title '" << window.title << "', pid="
			<< window.pid << ", tid=" << window.tid << std::endl;
		return; // We failed to connect, just return
	}
	// std::cout << "Connected thread" << std::endl;
	SetActiveWindow(getWindowHWND(window, true));
	for (auto key : keys) {
		sendKey(key);
	}

	while (!disconnectThread(window)) {
		std::cerr << "Failed to disconnect from thread!!!" << std::endl;
	}
}

void WinAssist::sendMouseEvents(WinInfo_t window, std::vector<MouseEvent> events) {
	// Attempt to connect to the window
	if (!connectToThread(window)) {
		std::wcerr << "Failed to send mouse events: unable to connect to thread of window with title '" << window.title << "', pid="
			<< window.pid << ", tid=" << window.tid << std::endl;
		return; // We failed to connect, just return
	}
	// std::cout << "Connected thread" << std::endl;
	SetActiveWindow(getWindowHWND(window, true));
	for (auto evt : events) {
		sendMouse(evt);
	}

	while (!disconnectThread(window)) {
		std::cerr << "Failed to disconnect from thread!!!" << std::endl;
	}
}

HWND WinAssist::getWindowHWND(WinInfo_t& window, bool allowUpdate) {
	HWND hwnd = FindWindow(NULL, window.title.c_str());
	DWORD pid;
	DWORD tid = GetWindowThreadProcessId(hwnd, &pid);
	if (pid == window.pid || tid == window.tid) {
		if(checkWinHwndValidity(hwnd))
			return hwnd;
	}
	std::wcerr << "Unable to find window with title " << window.title;
	std::wcerr << " with matching id (pid=" << pid << " tpid=" << window.pid << ", tid=" << tid << " ttid=" << window.tid << std::endl;

	// If we have update on, try to update the struct
	if (allowUpdate) {
		std::cout << "Attempting to update information: ";
		std::vector<WinInfo_t> winList = getWindowList();
		for (auto w : winList) {
			if (STD_WSTRING_CONTAINS(w.title, window.title)) {
				std::cout << "Match found by window title" << std::endl;
				window = w;
				return getWindowHWND(window);
			}
			else if (w.pid == window.pid) {
				std::cout << "Match found by pid" << std::endl;
				window = w;
				return getWindowHWND(window);
			}
			else if (w.tid == window.tid) {
				std::cout << "Match found by tid" << std::endl;
				window = w;
				return getWindowHWND(window);
			}
		}
	}

	return 0;
}

WinDimensions_t WinAssist::getWindowDimensions(WinInfo_t window) {
	WinDimensions_t dims;
	dims.topLeft = std::make_tuple(0, 0);
	dims.bottomRight = std::make_tuple(0, 0);
	dims.width = 0;
	dims.height = 0;
	RECT dimensions;

	if (GetWindowRect(getWindowHWND(window), &dimensions)) {
		dims.topLeft = std::make_tuple(dimensions.left, dimensions.top);
		dims.bottomRight = std::make_tuple(dimensions.right, dimensions.bottom);
		dims.width = dimensions.right - dimensions.left;
		dims.height = dimensions.bottom - dimensions.top;
	}
	else {
		std::wcerr << "Failed to get window dimensions for '" << window.title << "' (pid=" << window.pid << " tid=" << window.tid << ")" << std::endl;
	}

	return dims;
}