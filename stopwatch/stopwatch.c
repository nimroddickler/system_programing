#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <time.h>
#include <stdlib.h>


static volatile sig_atomic_t got_term; // initialize to 0
static volatile sig_atomic_t got_usr1; // initialize to 0
static volatile sig_atomic_t got_usr2; // initialize to 0
static volatile sig_atomic_t got_int;  // initialize to 0

static void on_usr1(int sig) { 
    (void)sig; // silence warning
    got_usr1 = 1;
}
static void on_usr2(int sig) { 
    (void)sig; // silence warning
    got_usr2 = 1;
}
static void on_term(int sig) { 
    (void)sig; // silence warning
    got_term = 1;
}
static void on_int(int sig) { 
    (void)sig; // silence warning
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

int main(void) {
    int pid = getpid();
    printf("%d\n", pid);
    
    install_handler(SIGTERM, on_term);
    install_handler(SIGUSR1, on_usr1);
    install_handler(SIGUSR2, on_usr2);
    install_handler(SIGINT,  on_int);

    struct timespec start;
    clock_gettime(CLOCK_MONOTONIC, &start);
    int running = 1; // boolean
    double frozen = 0.0;


    for (;;)
        {
            pause(); // wait for signal
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
                    start = now;   /* restart this segment */
                    running = 1;
                    printf("resumed\n");
                }
                got_usr1 = 0;
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
        }
}
