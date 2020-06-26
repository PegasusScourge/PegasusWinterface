/*

PegasusWinterface

Class that provides the ability to send keyboard and mouse commands to 
WinAPI windows.

*/

#include "PegasusWinterface.h"

#include <string>
#include <sstream>
#include <iostream>

namespace pi = pinterface;
using namespace pi;
using std::wcout;
using std::cout;
using std::cerr;
using std::endl;

/*******************************************************************************
		class TimedMouseEvent, public
********************************************************************************/

TimedMouseEvent::TimedMouseEvent(std::vector<MouseEvent> evts, int delayBefore) {
	m_events = evts;
	m_delayBefore = delayBefore;
}

TimedMouseEvent::TimedMouseEvent(MouseEvent evt, int delayBefore) {
	m_events.push_back(evt);
	m_delayBefore = delayBefore;
}

std::vector<MouseEvent>& TimedMouseEvent::getEvents() {
	return m_events;
}

int TimedMouseEvent::delayBefore() {
	return m_delayBefore;
}

/*******************************************************************************
		class TimedKeyEvent, public
********************************************************************************/

TimedKeyEvent::TimedKeyEvent(std::vector<KeyEvent> evts, int delayBefore) {
	m_events = evts;
	m_delayBefore = delayBefore;
}

TimedKeyEvent::TimedKeyEvent(KeyEvent evt, int delayBefore) {
	m_events.push_back(evt);
	m_delayBefore = delayBefore;
}

std::vector<KeyEvent>& TimedKeyEvent::getEvents() {
	return m_events;
}

int TimedKeyEvent::delayBefore() {
	return m_delayBefore;
}

/*******************************************************************************
		class PegasusTimer, private
********************************************************************************/

bool PegasusTimer::INITIALIZED = false;
LARGE_INTEGER PegasusTimer::COUNTER_FREQUENCY;
double PegasusTimer::MS_PER_COUNT = 0.0;

void PegasusTimer::InitializeAPI() {
	if (!INITIALIZED) {
		if (!QueryPerformanceFrequency(&COUNTER_FREQUENCY)) {
			cerr << "FAILED TO INITIALISE THE PegasusTimer API!!!!!" << endl;
			return;
		}
		INITIALIZED = true;
		INT64 freq = COUNTER_FREQUENCY.QuadPart;
		MS_PER_COUNT = 1000.0 / (double)freq;
		cout << "Initialised PegasusTimer API: freq=" << freq << " with " << MS_PER_COUNT << " ms per count" << endl;
	}
}

int PegasusTimer::CountsToMS(INT64 counts) {
	if (counts < 0) {
		counts = 0 - counts; // Make counts always positive
	}

	double ms = (double)counts * MS_PER_COUNT;
	return (int)ms;
}

/*******************************************************************************
		class PegasusTimer, public
********************************************************************************/

PegasusTimer::PegasusTimer() {
	if (!PegasusTimer::INITIALIZED)
		PegasusTimer::InitializeAPI();
	restart();
}

void PegasusTimer::restart() {
	LARGE_INTEGER currentCount;
	if (!QueryPerformanceCounter(&currentCount)) {
		cerr << "PegasusTimer error when doing restart(): unable to query the performance counter" << endl;
		m_startCount = 0;
		return;
	}
	m_startCount = currentCount.QuadPart;
}

int PegasusTimer::getElapsedTimeAsMilliseconds() {
	LARGE_INTEGER currentCount;
	INT64 currentC;
	if (!QueryPerformanceCounter(&currentCount)) {
		cerr << "PegasusTimer error when doing getElapsedTimeAsMilliseconds(): unable to query the performance counter" << endl;
		currentC = 0;
	}
	else {
		currentC = currentCount.QuadPart;
	}
	return PegasusTimer::CountsToMS(currentC - m_startCount);
}

/*******************************************************************************
		class PegasusWinterface, public
********************************************************************************/

/* Constructor */
PegasusWinterface::PegasusWinterface() {
	m_bound = false;

	m_timingClockKey.restart();
	m_timingClockMouse.restart();
}

PegasusWinterface::~PegasusWinterface() {
	// Nothing
}

void PegasusWinterface::unbind() {
	m_bound = false;
}

WinInfo_t PegasusWinterface::getWinInfo() {
	return m_winInfo;
}

WinDimensions_t PegasusWinterface::getWindowDimensions() {
	return m_winDims;
}

bool PegasusWinterface::bind(DWORD processID) {
	// Count and list all the windows for us
	std::vector<pi::WinInfo_t> windows = pi::WinAssist::GetWindowList();

	for (auto window : windows) {
		if (window.pid == processID) {
			wcout << "Found window with pid " << window.pid << " (name='" << window.title << "'), binding..." << endl;
			m_winInfo = window;
			m_bound = true;
			update();
			return true;
		}
	}
	return false;
}

bool PegasusWinterface::bind(std::string& str) {
	std::wstringstream stream;
	stream << str.c_str();
	std::wstring s = stream.str();
	return bind(s);
}

bool PegasusWinterface::bind(std::wstring& str) {
	// Count and list all the windows for us
	std::vector<pi::WinInfo_t> windows = pi::WinAssist::GetWindowList();

	for (auto window : windows) {
		if (STD_WSTRING_CONTAINS(window.title, str)) {
			wcout << "Found window with name '" << window.title << "' (pid=" << window.pid << "), binding..." << endl;
			m_winInfo = window;
			m_bound = true;
			update();
			return true;
		}
	}
	return false;
}

void PegasusWinterface::setBlocking(bool block) {
	m_blocking = block;
}

bool PegasusWinterface::isBlocking() {
	return m_blocking;
}

bool PegasusWinterface::hasEventsInQueue() {
	return !m_keyBuffer.empty() || !m_mouseBuffer.empty();
}

void PegasusWinterface::tick() {
	if (!m_bound)
		return;

	// If we are in blocking mode, we don't need to process this
	if (m_blocking)
		return;

	// Check that we have an event left
	if (!m_keyBuffer.empty()) {
		/* KEY EVENTS */
		// If the right amount of time has elapsed since the last key was sent, we can send the next one
		TimedKeyEvent evt = m_keyBuffer.at(0);
		bool emtpy = false;
		while (!emtpy && m_timingClockKey.getElapsedTimeAsMilliseconds() >= evt.delayBefore()) {
			cout << "Non-blocking exec: ";
			// Execute the event
			WinAssist::SendKeys(m_winInfo, evt.getEvents());
			m_timingClockKey.restart();
			// Remove the event
			m_keyBuffer.erase(m_keyBuffer.begin()); // Erase this event
			// Check if we have another event, if we do, load it up to see if it can be done immediately
			if (m_keyBuffer.empty()) {
				emtpy = true;
			}
			else {
				evt = m_keyBuffer.at(0);
			}
		}
	}

	if (!m_mouseBuffer.empty()) {
		/* MOUSE EVENTS */
		TimedMouseEvent evt = m_mouseBuffer.at(0);
		bool emtpy = false;
		while (!emtpy && m_timingClockMouse.getElapsedTimeAsMilliseconds() >= evt.delayBefore()) {
			cout << "Non-blocking exec: ";
			// Execute the event
			WinAssist::SendMouseEvents(m_winInfo, evt.getEvents());
			m_timingClockMouse.restart();
			// Remove the event
			m_mouseBuffer.erase(m_mouseBuffer.begin()); // Erase this event
			// Check if we have another event, if we do, load it up to see if it can be done immediately
			if (m_mouseBuffer.empty()) {
				emtpy = true;
			}
			else {
				evt = m_mouseBuffer.at(0);
			}
		}
	}
	
}

void PegasusWinterface::executeKeys(std::vector<TimedKeyEvent> keys, bool appendToQueue) {
	if (!m_bound)
		return;
	cout << "Executing/scheduling " << keys.size() << " timed key events" << endl;
	m_timingClockKey.restart();
	if (m_blocking) {
		// Process the keys here immediately and wait as necessary
		for (auto evt : keys) {
			// Wait until the key can be pressed
			while (m_timingClockKey.getElapsedTimeAsMilliseconds() < evt.delayBefore());
			// Execute the event
			WinAssist::SendKeys(m_winInfo, evt.getEvents());
			m_timingClockKey.restart();
		}
	}
	else {
		if (!appendToQueue) {
			m_keyBuffer.clear();
		}
		for (auto evt : keys) {
			m_keyBuffer.push_back(evt);
		}
	}
}

void PegasusWinterface::executeMouse(std::vector<TimedMouseEvent> evts, bool appendToQueue) {
	if (!m_bound)
		return;
	cout << "Executing/scheduling " << evts.size() << " timed mouse events" << endl;
	m_timingClockMouse.restart();
	if (m_blocking) {
		// Process the keys here immediately and wait as necessary
		for (auto evt : evts) {
			// Wait until the key can be pressed
			while (m_timingClockMouse.getElapsedTimeAsMilliseconds() < evt.delayBefore());
			// Execute the event
			WinAssist::SendMouseEvents(m_winInfo, evt.getEvents());
			m_timingClockMouse.restart();
		}
	}
	else {
		if (!appendToQueue) {
			m_mouseBuffer.clear();
		}
		for (auto evt : evts) {
			m_mouseBuffer.push_back(evt);
		}
	}
}


void PegasusWinterface::update() {
	if (!m_bound)
		return;

	m_winDims = WinAssist::GetWindowDimensions(m_winInfo);
}