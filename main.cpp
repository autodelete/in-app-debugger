// EXAMPLE APPLICATION

#include <iostream>
#include <unistd.h>
#include <stdio.h>
#include <thread>
#include <sys/types.h>
#include <sys/syscall.h>
#include <pthread.h>
#include <unistd.h>
#include "InAppDebugger.h"
#include "InAppDebuggerCallbacks.h"


using namespace std;

class Callbacks : public InAppDebuggerCallbacks {
    void logMessage(const std::string& msg) {
        printf("%d> [callbacks] %s\n", syscall(SYS_gettid), msg.c_str());
    }
};

void thread_bar(int x)
{
    printf("%d> inside thread_bar(%d)\n", syscall(SYS_gettid), x);
}

void thread_foo()
{
    printf("ww\n");
    printf("%d> inside thread_foo()\n", syscall(SYS_gettid));
    thread(thread_bar, 2).join();
    printf("%d> joined thread_bar\n", syscall(SYS_gettid));
}

int main() {
    printf("%d> Example app started\n", syscall(SYS_gettid));
    if (fork()) {
        InAppDebugger::enablePtraceForThisProcess();
        printf("%d> inside parent, sleeping 1 sec\n", syscall(SYS_gettid));
        sleep(1);
        printf("%d> printing stuff\n", syscall(SYS_gettid));
        thread(thread_foo).join();
        printf("zz\n");
        printf("%d> joined thread_foo\n", syscall(SYS_gettid));
        if (fork()) {
            printf("%d> after 2nd fork parent\n", syscall(SYS_gettid));
            sleep(2);
            printf("%d> going to long sleep\n", syscall(SYS_gettid));
            for(;;) {}
            printf("%d> woke up\n", syscall(SYS_gettid));
        } else {
            printf("%d> after 2nd fork child\n", syscall(SYS_gettid));
            thread(thread_foo).join();
            printf("%d> forked child done\n", syscall(SYS_gettid));
            sleep(20000);
        }
        sleep(1);
    } else {
        pid_t parentPid = getppid();
        printf("%d> inside child, parent pid=%d\n", syscall(SYS_gettid), parentPid);
        Callbacks callbacks;
        InAppDebugger debugger(&callbacks);
        debugger.logAllSyscalls();
        //debugger.logSyscall(1);
        debugger.attachToProcess(parentPid);
        debugger.run();
        sleep(1);
    }
    return 0;
}