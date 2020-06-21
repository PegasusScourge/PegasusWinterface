/*

InputSimulatorV1

Testing program for the PegasusWinterface system

*/

#include <iostream>
#include <vector>
#include <string>
#include <sstream>

#include "PegasusWinterface.h"
#include "WinAssist.h"
#include "ExtraKeyCodes.h"

namespace pi = pinterface;
using std::cout;
using std::cerr;
using std::wcerr;
using std::wcout;
using std::endl;

int main(int argc, char* argv[]) {
    cout << "InputSimulatorV1 : PegasusWinterface system test program" << endl;

    if (argc == 1) {
        // We need more than one argument so we know what to search for in the window title
        cerr << "argc == 1, need argument for window title search" << endl;
        return EXIT_FAILURE;
    }
    std::wstringstream stream;
    stream << argv[1];
    std::wstring windowSearch = stream.str();

    pi::PegasusWinterface app;
    if (!app.bind(windowSearch)) {
        wcerr << "Unable to find a window title containing '" << windowSearch << "' to bind!" << endl;
        return EXIT_FAILURE;
    }

    pi::WinInfo_t info = app.getWinInfo();
    wcout << "Got window with a title of '" << info.title << "', pid=" << info.pid << ", tid=" 
          << info.tid << ", isVisible=" << info.isVisible << endl;
    pi::WinDimensions_t dims = app.getWindowDimensions();
    wcout << "At (" << std::get<0>(dims.topLeft) << "," << std::get<1>(dims.topLeft) << "), width=" 
          << dims.width << " height=" << dims.height << endl;

    cout << "Creating key commands now..." << endl;

    std::vector<pi::TimedKeyEvent> kEvents;
    int keyOutput[] = {
        VK_LOWER_H,
        VK_LOWER_E,
        VK_LOWER_L,
        VK_LOWER_L,
        VK_LOWER_O,
        VK_SPACE,
        VK_LOWER_T,
        VK_LOWER_H,
        VK_LOWER_E,
        VK_LOWER_R,
        VK_LOWER_E,
    };

    for (int c : keyOutput) {
        pi::TimedKeyEvent evt(pi::KeyEvent(c), 250 /* delay in ms */);
        kEvents.push_back(evt);
    }
    kEvents.push_back(pi::KeyEvent(VK_RETURN));

    cout << "Sending keys (blocking):" << endl;
    app.setBlocking(true);
    app.executeKeys(kEvents);
    cout << "Done" << endl;

    cout << "Sending keys (non-blocking):" << endl;
    app.setBlocking(false);
    app.executeKeys(kEvents);
    while (app.hasEventsInQueue()) {
        // cout << "Ticking" << endl;
        app.tick();
    }
    cout << "Done" << endl;

    cout << "Sending mouse event" << endl;
    std::vector<pi::TimedMouseEvent> mEvents;
    pi::MouseEvent mevt = pi::MouseEvent(pi::MouseEvent::EventType::MEVT_KEY_PRESSED, pi::MouseEvent::MouseKey::MKEY_RIGHT);
    mEvents.push_back(pi::TimedMouseEvent(mevt, 300));
    mevt = pi::MouseEvent(pi::MouseEvent::EventType::MEVT_MOVE);
    mevt.setMoveValues(20, 90);
    mEvents.push_back(pi::TimedMouseEvent(mevt, 300));
    mevt = pi::MouseEvent(pi::MouseEvent::EventType::MEVT_KEY_PRESSED, pi::MouseEvent::MouseKey::MKEY_LEFT);
    mEvents.push_back(pi::TimedMouseEvent(mevt, 300));
    mevt = pi::MouseEvent(pi::MouseEvent::EventType::MEVT_KEY_PRESSED, pi::MouseEvent::MouseKey::MKEY_RIGHT);
    mEvents.push_back(pi::TimedMouseEvent(mevt, 300));
    mevt = pi::MouseEvent(pi::MouseEvent::EventType::MEVT_MOVE);
    mevt.setMoveValues(20, 70);
    mEvents.push_back(pi::TimedMouseEvent(mevt, 300));
    mevt = pi::MouseEvent(pi::MouseEvent::EventType::MEVT_KEY_PRESSED, pi::MouseEvent::MouseKey::MKEY_LEFT);
    mEvents.push_back(pi::TimedMouseEvent(mevt, 300));

    app.setBlocking(true);
    app.executeMouse(mEvents);

    while (dims.height > 300) {
        app.update();
        dims = app.getWindowDimensions();
        wcout << "At (" << std::get<0>(dims.topLeft) << "," << std::get<1>(dims.topLeft) << "), width="
            << dims.width << " height=" << dims.height << endl;
    }

    return EXIT_SUCCESS;
}