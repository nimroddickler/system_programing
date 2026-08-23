#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

static volatile sig_atomic_t got_term;
static volatile sig_atomic_t got_usr1;
static volatile sig_atomic_t got_usr2;
static volatile sig_atomic_t got_int;

static void on_usr1(int sig)
{
    (void)sig;
    got_usr1 = 1;
}

static void on_usr2(int sig)
{
    (void)sig;
    got_usr2 = 1;
}

static void on_term(int sig)
{
    (void)sig;
    got_term = 1;
}

static void on_int(int sig)
{
    (void)sig;
    got_int = 1;
}

static void install_handler(int signum, void (*handler)(int))
{
    struct sigaction sa;
    sa.sa_handler = handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    if (sigaction(signum, &sa, NULL) != 0) {
        perror("sigaction");
        exit(EXIT_FAILURE);
    }
}

int main(void)
{
    int pid = getpid();
    printf("%d\n", pid);

    install_handler(SIGTERM, on_term);
    install_handler(SIGUSR1, on_usr1);
    install_handler(SIGUSR2, on_usr2);
    install_handler(SIGINT,  on_int);

    sigset_t mask, oldmask, waitmask;
    sigemptyset(&mask);
    sigaddset(&mask, SIGUSR1);
    sigaddset(&mask, SIGUSR2);
    sigaddset(&mask, SIGINT);
    sigaddset(&mask, SIGTERM);
    if (sigprocmask(SIG_BLOCK, &mask, &oldmask) != 0) {
        perror("sigprocmask");
        exit(EXIT_FAILURE);
    }

    struct timespec start;
    clock_gettime(CLOCK_MONOTONIC, &start);
    int running = 1;
    double frozen = 0.0;

    for (;;) {
        if (sigprocmask(SIG_BLOCK, &mask, NULL) != 0) {
            perror("sigprocmask");
            exit(EXIT_FAILURE);
        }

        if (got_usr2) {
            struct timespec now;
            clock_gettime(CLOCK_MONOTONIC, &now);
            double slice = (now.tv_sec - start.tv_sec)
                         + (now.tv_nsec - start.tv_nsec) / 1e9;
            double elapsed = running ? frozen + slice : frozen;
            printf("elapsed: %.3f s\n", elapsed);
            got_usr2 = 0;
        }
        if (got_usr1) {
            struct timespec now;
            clock_gettime(CLOCK_MONOTONIC, &now);
            double slice = (now.tv_sec - start.tv_sec)
                         + (now.tv_nsec - start.tv_nsec) / 1e9;

            if (running) {
                frozen += slice;
                running = 0;
                printf("paused\n");
            } else {
                start = now;
                running = 1;
                printf("resumed\n");
            }
            got_usr1 = 0;
        }
        if (got_int) {
            frozen = 0.0;
            clock_gettime(CLOCK_MONOTONIC, &start);
            running = 1;
            printf("reset\n");
            got_int = 0;
        }
        if (got_term) {
            printf("terminated cleanly\n");
            break;
        }

        waitmask = oldmask;
        sigdelset(&waitmask, SIGUSR1);
        sigdelset(&waitmask, SIGUSR2);
        sigdelset(&waitmask, SIGINT);
        sigdelset(&waitmask, SIGTERM);
        sigsuspend(&waitmask);
    }
}
