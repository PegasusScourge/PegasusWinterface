#pragma once
/*

WinAssist

Static class that provides utility functions for getting windows API information, sending keys etc

*/

#include <vector>
#include <string>
#include <tuple>

#include <Windows.h>

#define STD_WSTRING_CONTAINS(a, b) (a.find(b) != std::wstring::npos)

namespace pinterface {

/*******************************************************************************
		struct WinInfo
********************************************************************************/
	typedef struct WinInfo {
		std::wstring title;
		bool isVisible;
		DWORD pid; // Process ID
		DWORD tid; // Thread ID
	} WinInfo_t;

/*******************************************************************************
		struct WinDimensions
********************************************************************************/
	typedef struct WinDimensions {
		std::tuple<LONG, LONG> topLeft;
		std::tuple<LONG, LONG> bottomRight;
		LONG width;
		LONG height;
	} WinDimensions_t;

	class KeyEvent {
/*******************************************************************************
		class KeyEvent, public
********************************************************************************/
	public:
		enum class EventType { KEVT_NONE, KEVT_TYPED, KEVT_PRESSED, KEVT_RELEASED };
		KeyEvent(WORD vKey, EventType type = EventType::KEVT_TYPED, bool scanCode = true, bool isExtended = false);
		KeyEvent();
		WORD vKey();
		bool scanCode();
		bool isExtended();
		EventType type();

/*******************************************************************************
		class KeyEvent, private
********************************************************************************/
	private:
		WORD m_vKey;
		bool m_isExtended;
		bool m_scanCode;
		enum EventType m_type;
	};

	class MouseEvent {
/*******************************************************************************
		class MouseEvent, public
********************************************************************************/
	public:
		enum class EventType { MEVT_NONE, MEVT_MOVE, MEVT_MOVE_ABS, MEVT_MOVE_DESKTOP, MEVT_SCROLL, MEVT_KEY_PRESSED, MEVT_KEY_DOWN, MEVT_KEY_UP };
		enum class MouseKey { MKEY_NONE, MKEY_LEFT, MKEY_RIGHT, MKEY_MID };
		MouseEvent(EventType type, MouseKey vKey = MouseKey::MKEY_NONE);
		MouseEvent();

		void setMoveValues(LONG dx, LONG dy);
		void setScrollDelta(DWORD d);

		EventType type();
		MouseKey key();
		LONG dx();
		LONG dy();
		DWORD scrollDelta();

/*******************************************************************************
		class MouseEvent, private
********************************************************************************/
	private:
		enum EventType m_type;
		enum MouseKey m_key;
		LONG m_dx = 0;
		LONG m_dy = 0;
		DWORD m_scrollDelta = 0;
	};

/*******************************************************************************
		namespace WinCallbacks
********************************************************************************/
	namespace WinCallbacks {
		// Callback function that is called per window to return the information about the window.
		// Requires a std::vector<WinCallbacks::WinInfo_t>* to be passed as lParam
		BOOL CALLBACK winInfoList(HWND hwnd, LPARAM lParam); 
	}

	class WinAssist {
/*******************************************************************************
		class WinAssist, private
********************************************************************************/
	private:
		/* Private static variables */
		static std::vector<WinInfo_t> WIN_INFO;

		/* Private static functions */
		// Updates the names in the WIN_NAMES list to the currently open applications
		static void UpdateWinNames();

		// Checks the validity of the handle passed to it. WARNING: handle reuse may give the indication nothing has changed, further
		// the window may change state immediately after the test. DO NOT USE AS A GUARANTEE, just an indication
		static bool CheckWinHwndValidity(HWND hwnd);

		// Connects this thread to another thread
		static bool	ConnectToThread(WinInfo_t window);

		// Disconnects this thread from another thread
		static bool DisconnectThread(WinInfo_t window);

		// Sends a key to another thread. MUST BE CONNECTED BEFORE DOING SO
		static void SendKey(KeyEvent key);

		// Sends a mouse event to another thread. MUST BE CONNECTED BEFORE DOING SO
		static void SendMouse(MouseEvent evt);

		// Sends a key to another thread. Doesn't need connection before doing so. (PostThreadMessage method)
		// DEPRECATED
		// static void sendKeyB(WinInfo_t window, WORD key);

		// Sends many keys using sendKeyB (PostThreadMessage method) KeyEvent
		// DEPRECATED
		// static void sendKeysB(WinInfo_t window, std::vector<WORD> keys);

/*******************************************************************************
		class WinAssist, public
********************************************************************************/
	public:
		/* Public static functions */
		static std::vector<WinInfo_t> GetWindowList();
		static std::vector<WinInfo_t> GetVisibleWindowList();
		// Sends many keys
		static void SendKeys(WinInfo_t window, std::vector<KeyEvent> keys);
		// Sends many mouse events
		static void SendMouseEvents(WinInfo_t window, std::vector<MouseEvent> events);
		// Gets the window HWND from windows using the window information. If allow update is set, will update the info
		// struct if the window can't be found
		static HWND GetWindowHWND(WinInfo_t& window, bool allowUpdate = false);
		// Returns the dimensions of a window from its window handle 
		static WinDimensions_t GetWindowDimensions(WinInfo_t window);
	};

}