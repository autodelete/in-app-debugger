#include "Thread.h"
#include "InAppDebugger.h"
#include "syscall_abi.h"

#include <sys/user.h>
#include <sys/ptrace.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <assert.h>
#include <errno.h>

using namespace std;


Thread::Thread(InAppDebugger*dbg, pid_t tid) : dbg(dbg), tid(tid) {
}

Thread::Thread(InAppDebugger*dbg, pid_t tid, shared_ptr<Process> owningProcess) : dbg(dbg), tid(tid), process(owningProcess) {
}

void Thread::setOwningProcess(shared_ptr<Process> process) {
    this->process = process;
}

void Thread::processEvent(int status) {
    int pass_sig = 0;
    dbg->log("%d: processing event 0x%04x (%d)", tid, status, status);
    if (status >> 8 == (SIGTRAP | (PTRACE_EVENT_EXIT << 8))) {
        dbg->log("%d: thread exited", tid);
        dbg->removeThread(tid);
        handleClone();
        return;
    }
    if (status >> 8 == (SIGTRAP | (PTRACE_EVENT_CLONE << 8))) {
        handleClone();
        return;
    }
    if (status >> 8 == (SIGTRAP | (PTRACE_EVENT_FORK << 8))) {
        handleFork();
        return;
    }
    if (status >> 8 == (SIGTRAP | (PTRACE_EVENT_FORK << 8))) {
        handleFork();
        return;
    }
    if (status >> 8 == (SIGTRAP | 0x80)) {
        user_regs_struct regs;
        ptrace(PTRACE_GETREGS, tid, 0, &regs);

        switch (state) {
            case State::WAITING_SYSCALL_ENTER:
                if (dbg->needLogSyscall(regs.REG_SYSCALL_NO)) {
                    dbg->log("%d: enter syscall_%ld(%ld, %ld, %ld, %ld, %ld, %ld)",
                             tid,
                             (long) regs.REG_SYSCALL_NO,
                             (long) regs.REG_SYSCALL_ARG1,
                             (long) regs.REG_SYSCALL_ARG2,
                             (long) regs.REG_SYSCALL_ARG3,
                             (long) regs.REG_SYSCALL_ARG4,
                             (long) regs.REG_SYSCALL_ARG5,
                             (long) regs.REG_SYSCALL_ARG6);
                }
                state = State::WAITING_SYSCALL_LEAVE;
                break;
            case State::WAITING_SYSCALL_LEAVE:
                if (dbg->needLogSyscall(regs.REG_SYSCALL_NO)) {
                    dbg->log("%d: leave syscall_%ld() -> %ld", tid,
                            (long) regs.REG_SYSCALL_NO,
                            (long) regs.REG_SYSCALL_RETVAL);
                }
                state = State::WAITING_SYSCALL_ENTER;
                break;
        }
    } else if (status == 0) {
        dbg->log("%d: thread exited", tid);
        dbg->removeThread(tid);
        return;
    } else {
        dbg->log("%d: received signal %d, passing to app", tid, status >> 8);
        pass_sig = status >> 8;
    }
    ptraceResume(pass_sig);
}

void Thread::ptraceResume(int signal) {
    dbg->log("%d: resuming with signal %d", tid, signal);
    if (0 != ptrace(PTRACE_SYSCALL, tid, 0, signal)) {
        dbg->log("%d: ERROR: failed to resume thread with signal=%d: errno=%d", tid, signal, errno);
    }
}

void Thread::handleClone() {
    unsigned long event_message = 0;
    ptrace(PTRACE_GETEVENTMSG, tid, nullptr, &event_message);
    dbg->log("%d: started new thread %d", tid, event_message);
//
//    if (0 != ptrace(PTRACE_CONT, tid, 0, 0)) {
//        dbg->log("%d: ERROR: failed to resume thread with signal=%d: errno=%d", tid, signal, 0);
//    }
//
//    if (0 != ptrace(PTRACE_CONT, event_message, 0, 0)) {
//        dbg->log("%d: ERROR: failed to resume thread with signal=%d: errno=%d", event_message, 0, errno);
//    }
//
  //  dbg->onNewThread(process.lock(), event_message, 1);

    // Registering new thread with the debugger
    dbg->onNewThread(process.lock(), event_message, (SIGTRAP|0x80)<<8);
    // resuming this thread immediately
    ptraceResume(0);
}

void Thread::handleFork() {
    unsigned long event_message = 0;
    ptrace(PTRACE_GETEVENTMSG, tid, nullptr, &event_message);
    dbg->log("%d: forked new process %d", tid, event_message);
    // Registering new thread with the debugger
    dbg->onNewProcess(process.lock(), event_message, (SIGTRAP|0x80)<<8);
    // resuming this thread immediately
    ptraceResume(0);
}
