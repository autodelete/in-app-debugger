#ifndef IN_APP_DEBUGGER_INAPPDEBUGGER_H
#define IN_APP_DEBUGGER_INAPPDEBUGGER_H

#include <memory>
#include <unordered_map>
#include <unordered_set>
#include <queue>
#include "InAppDebuggerCallbacks.h"
#include "Process.h"

class InAppDebugger {
public:
    /// Enables ptracing of the current process from the outside
    static void enablePtraceForThisProcess();

    /// Constructs the in-app debugger
    InAppDebugger(InAppDebuggerCallbacks* callbackInterface);

    /// Attaches to a given target process. Process will be stopped as a result.
    std::shared_ptr<Process> attachToProcess(pid_t targetPid);

    void run();
    void logAllSyscalls();
    void logSyscall(int syscallNo);

private:
    InAppDebuggerCallbacks* callbackInterface;

    void log(const char *format, ...);
    void processEvents();
    void removeThread(const pid_t i);
    void onNewThread(std::shared_ptr<Process> owningProcess, pid_t tid, int status);
    bool needLogSyscall(int syscallNo);

    std::unordered_map<pid_t, std::shared_ptr<Thread>> threads;
    std::unordered_map<pid_t, std::shared_ptr<Process>> processes;
    std::queue<std::pair<std::shared_ptr<Thread>,int>> processing_queue;

    friend class Process;

    friend class Thread;
    bool doLogAllSyscalls = false;

    std::unordered_set<int> syscallsToLog;

    void onNewProcess(std::shared_ptr<Process> parentProcess, unsigned long pid, int status);

};

#endif //IN_APP_DEBUGGER_INAPPDEBUGGER_H
