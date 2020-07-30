#include "InAppDebugger.h"
#include "Thread.h"
#include "Process.h"
#include <sstream>
#include <stdarg.h>
#include <memory>
#include <assert.h>
#include <sys/ptrace.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <sys/user.h>
#include <sys/reg.h>
#include <sys/syscall.h>
#include <stdio.h>
#include <sys/ptrace.h>
#include <pthread.h>
#include <sys/prctl.h>
#include <vector>
#include <errno.h>


using namespace std;

void InAppDebugger::enablePtraceForThisProcess() {
    prctl(PR_SET_PTRACER, PR_SET_PTRACER_ANY, 0, 0, 0, 0);
}

InAppDebugger::InAppDebugger(InAppDebuggerCallbacks *callbackInterface)
    :callbackInterface(callbackInterface) {
    callbackInterface->logMessage("In-app debugger initialized");
}

shared_ptr<Process> InAppDebugger::attachToProcess(pid_t targetPid) {
    log("Attaching to process %d", targetPid);
    shared_ptr<Process> process = Process::Create(this, targetPid);
    int x = ptrace(PTRACE_ATTACH, targetPid);
    if (x != 0) {
        log("Failed to ptrace(PTRACE_ATTACH, %d), errno=%d", targetPid, errno);
        return process;
    }
    int status;
    int pid = waitpid(targetPid, &status, 0);
    if (pid != targetPid) {
        log("Failed to stop %d, errno=%d", targetPid, errno);
        return process;
    }
    x = ptrace(PTRACE_SETOPTIONS, targetPid, nullptr,
            PTRACE_O_TRACECLONE|
            PTRACE_O_TRACEFORK|
            PTRACE_O_TRACESYSGOOD);
    if (x != 0) {
        log("Failed to ptrace(PTRACE_SETOPTIONS, %d), errno=%d", targetPid, errno);
        return process;
    }
    log("Successfully attached to process %d", targetPid);
    processes[targetPid] = process;
    threads[targetPid] = process->getMainThread();
    processing_queue.push({process->getMainThread(), status});
    return process;
}

void InAppDebugger::log(const char *format, ...) {
    char buffer[1024];
    va_list args;
    va_start (args, format);
    vsnprintf (buffer,sizeof(buffer),format, args);
    va_end (args);
    callbackInterface->logMessage(buffer);
}

void InAppDebugger::run() {
    log("Entering debugger run loop");
    while (threads.size() > 0) {
        processEvents();
        int status = 0;
        pid_t tid = wait4(-1, &status, __WALL, NULL);
        auto it = threads.find(tid);
        if (it == threads.end()) {
            log("Received event 0x%04x from unknown (yet?) thread or process: %d, continuing it", status, tid);
            ptrace(PTRACE_SYSCALL,tid,0,0);
            continue;
        } else {
            log("Received event 0x%04x for %d", status, tid);
        }
        processing_queue.push({it->second, status});
    }
    log("No more processes/threads under debugging, exiting debugger run loop");
}

void InAppDebugger::processEvents() {
    while (!processing_queue.empty()) {
        queue<pair<shared_ptr<Thread>, int>> queue;
        queue.swap(processing_queue);
        while (!queue.empty()) {
            queue.front().first->processEvent(queue.front().second);
            queue.pop();
        }
    }
}

void InAppDebugger::removeThread(const pid_t tid) {
    threads.erase(tid);
    processes.erase(tid);
}

bool InAppDebugger::needLogSyscall(int syscallNo) {
    syscallNo = ((unsigned int)syscallNo) & 0xFFFF;
    return doLogAllSyscalls || syscallsToLog.find(syscallNo) != syscallsToLog.end();
}

void InAppDebugger::logAllSyscalls() {
    doLogAllSyscalls = true;
}

void InAppDebugger::logSyscall(int syscallNo) {
    syscallsToLog.insert(syscallNo);
}

void InAppDebugger::onNewThread(shared_ptr<Process> owningProcess, pid_t tid, int status) {
    shared_ptr<Thread> thread = make_shared<Thread>(this, tid, owningProcess);
    thread->state = Thread::State::WAITING_SYSCALL_LEAVE;
    threads[tid] = thread;
    //processing_queue.push({thread, status});
}

void InAppDebugger::onNewProcess(shared_ptr<Process> parentProcess, unsigned long pid, int status) {
    shared_ptr<Process> process = Process::Create(this, pid);
    threads[pid] = process->mainThread;
    process->mainThread->state = Thread::State::WAITING_SYSCALL_LEAVE;
    //processing_queue.push({process->mainThread, status});
}

