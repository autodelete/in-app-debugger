#ifndef IN_APP_DEBUGGER_INAPPDEBUGGERCALLBACKS_H
#define IN_APP_DEBUGGER_INAPPDEBUGGERCALLBACKS_H

#include <string>

/// Interface to be implemented by the application
class InAppDebuggerCallbacks {
public:
    /// Called when debugger wants to log message
    virtual void logMessage(const std::string &msg) = 0;
};

#endif //IN_APP_DEBUGGER_INAPPDEBUGGERCALLBACKS_H
