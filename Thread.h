#ifndef IN_APP_DEBUGGER_THREAD_H
#define IN_APP_DEBUGGER_THREAD_H

#include <fcntl.h>
#include <memory>

class Process;
class InAppDebugger;

class Thread {

public:
    Thread(InAppDebugger* dbg, pid_t tid, std::shared_ptr<Process> owningProcess);
private:
    void processEvent(int status);
    void setOwningProcess(std::shared_ptr<Process> process);
    Thread(InAppDebugger* dbg, pid_t tid);

    InAppDebugger* dbg;
    const pid_t tid;
    std::weak_ptr<Process> process;
    friend class Process;
    friend class InAppDebugger;

    enum class State {
        WAITING_SYSCALL_ENTER,
        WAITING_SYSCALL_LEAVE,
    };
    State state = State::WAITING_SYSCALL_ENTER;

    void handleClone();

    void handleFork();

    void ptraceResume(int signal);
};


#endif //IN_APP_DEBUGGER_THREAD_H
