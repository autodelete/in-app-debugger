#include "Process.h"
#include "Thread.h"

using namespace std;
Process::Process(InAppDebugger* dbg, pid_t pid, std::shared_ptr<Thread> mainThread)
    : dbg(dbg), pid(pid), mainThread(mainThread)
{

}

std::shared_ptr<Process> Process::Create(InAppDebugger* dbg, pid_t pid) {
    auto thread = shared_ptr<Thread>(new Thread(dbg, pid));
    auto process = shared_ptr<Process>(new
            Process(dbg, pid,thread));
    thread->setOwningProcess(process);
    return process;
}


