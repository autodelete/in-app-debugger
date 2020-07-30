#ifndef IN_APP_DEBUGGER_PROCESS_H
#define IN_APP_DEBUGGER_PROCESS_H

#include <memory>
#include "Thread.h"
#include "InAppDebugger.h"

class Thread;
class InAppDebugger;

class Process {

public:
    std::shared_ptr<Thread> getMainThread() { return mainThread; }
    static std::shared_ptr<Process> Create(InAppDebugger* dbg, pid_t pid);

private:
    Process(InAppDebugger* dbg, pid_t pid, std::shared_ptr<Thread> mainThread);

    InAppDebugger* dbg;
    const pid_t pid;
    std::shared_ptr<Thread> mainThread;

    friend class InAppDebugger;
};


#endif //IN_APP_DEBUGGER_PROCESS_H
