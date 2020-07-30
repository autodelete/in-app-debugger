#ifndef IN_APP_DEBUGGER_PLATFORM_SPECIFIC_H
#define IN_APP_DEBUGGER_PLATFORM_SPECIFIC_H


// See 'man 2 syscall' for more information

#if __amd64__
#define REG_SYSCALL_NO orig_rax
#define REG_SYSCALL_RETVAL rax
#define REG_SYSCALL_ARG1 rdi
#define REG_SYSCALL_ARG2 rsi
#define REG_SYSCALL_ARG3 rdx
#define REG_SYSCALL_ARG4 r10
#define REG_SYSCALL_ARG5 r8
#define REG_SYSCALL_ARG6 r9

#elif __i386__
#define REG_SYSCALL_NO orig_eax
#define REG_SYSCALL_RETVAL eax
#define REG_SYSCALL_ARG1 ebx
#define REG_SYSCALL_ARG2 ecx
#define REG_SYSCALL_ARG3 edx
#define REG_SYSCALL_ARG4 esi
#define REG_SYSCALL_ARG5 edi
#define REG_SYSCALL_ARG6 ebp

#elif __arm__
#define REG_SYSCALL_NO uregs[7]
#define REG_SYSCALL_RETVAL uregs[0]
#define REG_SYSCALL_ARG1 uregs[0]
#define REG_SYSCALL_ARG2 uregs[1]
#define REG_SYSCALL_ARG3 uregs[2]
#define REG_SYSCALL_ARG4 uregs[3]
#define REG_SYSCALL_ARG5 uregs[4]
#define REG_SYSCALL_ARG6 uregs[5]

typedef user_regs user_regs_struct;

#else
#error Unsupported processor/architecture
#endif

#endif //IN_APP_DEBUGGER_PLATFORM_SPECIFIC_H
