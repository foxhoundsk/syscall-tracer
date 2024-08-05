#define _GNU_SOURCE
#include <stdlib.h>
#include <sys/ptrace.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/user.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/syscall.h>
#include <sys/uio.h>

/* aarch32 specific */
#define GET_SYSCALL_NUM(reg) (regs.uregs[7])
/* (532 + 4) bytes, the 4-byte seems like the padding for @gps_valid. */
#define ORIG_PKT_SZ 536
/* the data that is of intereset to us */
#define PKT_SZ 48

#define offsetof(TYPE, MEMBER)	((size_t)&((TYPE *)0)->MEMBER)

typedef struct {
    double gyro[10][3];
    double accel[10][3];

    /* GPSR */
    double posx;
    double posy;
    double posz;
    double velx;
    double vely;
    double velz;
    int gps_valid;
} navi_ingress_packet_t;

typedef struct {
    /* GPSR */
    double posx;
    double posy;
    double posz;
    double velx;
    double vely;
    double velz;
    int gps_valid;
} reduced_navi_ingress_packet_t;

int main(int ac, char **av)
{
    long ret;
    int status;
    char buf[PKT_SZ];
    int target_tick = 0;
    reduced_navi_ingress_packet_t *pkt = (reduced_navi_ingress_packet_t *) buf;
    struct iovec iov = {
        .iov_base = buf,
        .iov_len  = PKT_SZ,
    };

    struct iovec riov = {
        .iov_len  = PKT_SZ,
    };

    struct iovec dst_iov = {
        .iov_len  = PKT_SZ,
    };

    if (ac != 2) {
        puts("Too few arg.\n\nUsage ./fault_inj <target_tid>\n");
        exit(-1);
    }

    int pid = atoi(av[1]);

    printf("Tracing pid %d\n", pid);

    ret = ptrace(PTRACE_ATTACH, pid, NULL, NULL);
    if (ret != 0) {
        perror("Error attaching process");
        exit(-1);
    }

    /* Return of PTRACE_ATTACH doesn't mean that the process is ready to be
     * traced. Add a waitpid to ensure such thing. The next ptrace command
     * would possibly fail with "No such process" otherwise.
     */
    waitpid(pid, &status, 0);

    for (;;) {
        struct user_regs regs;
        void *r_addr;

        /* Enter next system call */
        if (ptrace(PTRACE_SYSCALL, pid, 0, 0) != 0) {
            perror("Error wait sysenter");
        }
        waitpid(pid, 0, 0);

        if (ptrace(PTRACE_GETREGS, pid, 0, &regs) != 0) {
            perror("Error get_regs for sysenter");
        }

        /* for syscalls that are out of interest, do fast path. */
        if (GET_SYSCALL_NUM(reg) != SYS_recvfrom) {
            ptrace(PTRACE_SYSCALL, pid, 0, 0);
            waitpid(pid, 0, 0);
            continue;
        }

        printf("%d\n", target_tick++);

        /* same as above */
        if (target_tick != 100) {
            ptrace(PTRACE_SYSCALL, pid, 0, 0);
            waitpid(pid, 0, 0);
            continue;
        }

        printf("syscall %ld requested, arg1: %ld, arg2: %ld\n",
                regs.uregs[7],
                regs.uregs[1],
                regs.uregs[2]
                );

        r_addr = (void *) regs.uregs[1];

        /* syscall exit */
        if (ptrace(PTRACE_SYSCALL, pid, 0, 0) != 0) {
            perror("Error wait sysexit");
        }
        waitpid(pid, 0, 0);

        if (ptrace(PTRACE_GETREGS, pid, 0, &regs) != 0) {
            perror("Error get_regs for sysret");
        }

        if (regs.uregs[0] == ORIG_PKT_SZ) {
            ssize_t ret;

            /* the src addr contains the original structure */
            riov.iov_base = r_addr + offsetof(navi_ingress_packet_t, posx);
            dst_iov.iov_base = riov.iov_base;

            ret = process_vm_readv(pid, &iov, 1, &riov, 1, 0);
            if (-1 == ret) {
                perror("Error process_vm_readv");
                continue;
            }

            /*
            printf("posx: %lf, posy: %lf, posz: %lf\n",
                    pkt->posx,
                    pkt->posy,
                    pkt->posz);*/

            /* tamper posx */
            pkt->posx += 5;

            ret = process_vm_writev(pid, &iov, 1, &dst_iov, 1, 0);
            if (-1 == ret || ret != iov.iov_len) {
                printf("ret=%d\n", ret);
                perror("Error process_vm_writev");
                continue;
            }

            puts("Tampering sucessed");

            break;
        } else {
            printf("Unknown pkt size: %ld\n", regs.uregs[0]);
        }
    }

    ret = ptrace(PTRACE_DETACH, pid, NULL, NULL);
    if (ret != 0) {
        perror("Error detaching process");
        exit(-1);
    }
    puts("Sucessfully detached process");


    return 0;
}
