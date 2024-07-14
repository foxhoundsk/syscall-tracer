#include <unistd.h>
#include <stdio.h>

int main()
{
    printf("pid: %d\n", getpid());

    while (1) {
        write(1, "hi", 2);
        sleep(1);
    }

    return 0;
}
