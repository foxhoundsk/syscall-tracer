#include <unistd.h>
#include <stdio.h>
#include <sys/syscall.h>

int main()
{
    printf("pid: %d\n", getpid());

    while (1) {
        recvfrom(0, 1234,  0,0,0,0);
        write(1, "hi", 3);
        sleep(1);
    }

    return 0;
}
