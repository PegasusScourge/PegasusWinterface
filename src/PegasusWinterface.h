#pragma once
/*

PegasusWinterface

Class that provides the ability to send keyboard and (hopefully at some point) mouse commands to
WinAPI windows.

*/

#include "WinAssist.h"
#include <SFML/System/Clock.hpp>

namespace pinterface {

	// TODO
	class TimedMouseEvent {
/*******************************************************************************
		class TimedMouseEvent, private
********************************************************************************/
		std::vector<MouseEvent> m_events;
		int m_delayBefore;

	public:
/*******************************************************************************
		class TimedMouseEvent, public
********************************************************************************/
		TimedMouseEvent(std::vector<MouseEvent> evts, int delayBefore = 0);
		TimedMouseEvent(MouseEvent evt, int delayBefore = 0);
		std::vector<MouseEvent>& getEvents();
		int delayBefore();
	};

	class TimedKeyEvent {

	private:
/*******************************************************************************
		class TimedKeyEvent, private
********************************************************************************/
		std::vector<KeyEvent> m_events;
		int m_delayBefore;

	public:
/*******************************************************************************
		class TimedKeyEvent, public
********************************************************************************/
		TimedKeyEvent(std::vector<KeyEvent> evts, int delayBefore = 0);
		TimedKeyEvent(KeyEvent evt, int delayBefore = 0);
		std::vector<KeyEvent>& getEvents();
		int delayBefore();
	};

	class PegasusWinterface {
/*******************************************************************************
		class PegasusWinterface, private
********************************************************************************/
	private:
		/* Private static variables */


		/* Private member variables */
		bool m_bound = false;
		bool m_blocking = false;
		sf::Clock m_timingClockKey;
		sf::Clock m_timingClockMouse;
		// unsigned int m_waitTime = 0;
		std::vector<TimedKeyEvent> m_keyBuffer;
		std::vector<TimedMouseEvent> m_mouseBuffer;

		WinInfo_t m_winInfo;
		WinDimensions_t m_winDims;

		/* Private member functions */

/*******************************************************************************
		class PegasusWinterface, public
********************************************************************************/
	public:
		PegasusWinterface();
		~PegasusWinterface();
		
		/* Public member functions */
		// Bind functions. Takes a string to search window titles for, or a process ID. Returns true if bind is successful
		bool bind(std::wstring& str);
		bool bind(std::string& str);
		bool bind(DWORD processID);
		// Returns the current status of the interface
		inline bool isBound() { return m_bound; }
		// Unbinds the interface
		void unbind();
		// Sets the blocking behaviour of the interface. If true, will execute the events when execute<EVENT> is called, instead
		// of scheduling to be executed if appropriate it the tick function
		void setBlocking(bool block);
		// Returns the current blocking status
		bool isBlocking();
		// Returns the information about the Window and process that is captured. Not a copy so cannot be modified
		WinInfo_t getWinInfo();
		// Returns the window dimension information
		WinDimensions_t getWindowDimensions();
		// Executes the current queue of events when appropriate according to their timing
		void tick();
		// Checks if there are events in the queues
		bool hasEventsInQueue();
		// Schedules or immediately executes key events
		void executeKeys(std::vector<TimedKeyEvent> keys, bool appendToQueue = false);
		// Schedules or immediately executes mouse events
		void executeMouse(std::vector<TimedMouseEvent> evts, bool appendToQueue = false);
		// Updates the information held by the interface
		void update();
	};

}