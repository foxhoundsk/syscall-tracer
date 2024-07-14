# So You Want to Trace Linux Syscall

The `tracer` intecepts the syscalls made by `tracee`, and shows the syscall number and the retval.

The tracer currently supports only `amd64`.

## Caveats
- Since `PTRACE_SYSCALL` stops the tracee process every time a syscall is made (signal is intercepted as well), apps that invoke plenty of syscalls may have degraded throughput when being traced.
