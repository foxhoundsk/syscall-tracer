#include <unistd.h>
#include <stdio.h>
#include <sys/ptrace.h>
#include <syscall.h>
#include <stdlib.h>
#include <sys/user.h>
#include <sys/wait.h>

int main(int ac, char **av)
{
    int pid;

    if (ac != 2) {
        printf("Usage: ./tracer <tracee_pid>\n");
        return -1;
    }
    pid = atoi(av[1]);

    printf("Attaching pid %d\n", pid);
    if (ptrace(PTRACE_ATTACH, pid, 0, 0) != 0) {
        perror("Error attaching");
        return -1;
    }
    printf("Successfully attached pid %d\n", pid);
        
    for (;;) {
        struct user_regs_struct regs;

        /* Enter next system call */
        ptrace(PTRACE_SYSCALL, pid, 0, 0);
        waitpid(pid, 0, 0);

        ptrace(PTRACE_GETREGS, pid, 0, &regs);
        
        printf("syscall %lld requested\n", regs.orig_rax);

        /* Run system call and stop on exit */
        ptrace(PTRACE_SYSCALL, pid, 0, 0);
        waitpid(pid, 0, 0);

        printf("syscall retval: %lld\n\n", regs.rax);
    }

    return 0;
}
