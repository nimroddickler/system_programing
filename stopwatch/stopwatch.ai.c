/* Signal-driven stopwatch — control with kill, no keyboard input */
#include <signal.h>
#include <stdio.h>
#include <time.h>
#include <unistd.h>

/* Flags set inside handlers; main loop reacts (async-signal-safe pattern) */
static volatile sig_atomic_t toggle_pause;
static volatile sig_atomic_t print_elapsed;
static volatile sig_atomic_t reset_timer;
static volatile sig_atomic_t terminate;

static void on_usr1(int sig) {
    (void)sig;
    toggle_pause = 1;
}

static void on_usr2(int sig) {
    (void)sig;
    print_elapsed = 1;
}

static void on_int(int sig) {
    (void)sig;
    reset_timer = 1;
}

static void on_term(int sig) {
    (void)sig;
    terminate = 1;
}

static double now_sec(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (double)ts.tv_sec + (double)ts.tv_nsec / 1e9;
}

/* Elapsed = frozen total while paused, else frozen + time since last resume */
static double elapsed_sec(int running, double start, double frozen) {
    if (running)
        return frozen + (now_sec() - start);
    return frozen;
}

int main(void) {
    struct sigaction sa;

    sa.sa_flags = 0;
    sigemptyset(&sa.sa_mask);

    sa.sa_handler = on_usr1;
    sigaction(SIGUSR1, &sa, NULL);

    sa.sa_handler = on_usr2;
    sigaction(SIGUSR2, &sa, NULL);

    sa.sa_handler = on_int;
    sigaction(SIGINT, &sa, NULL);

    sa.sa_handler = on_term;
    sigaction(SIGTERM, &sa, NULL);

    int running = 1;
    double start = now_sec();
    double frozen = 0.0;

    printf("stopwatch pid=%d  (running)\n", getpid());
    printf("  kill -USR1 %d   pause/resume\n", getpid());
    printf("  kill -USR2 %d   print elapsed\n", getpid());
    printf("  kill -INT  %d   reset\n", getpid());
    printf("  kill -TERM %d   exit\n", getpid());
    fflush(stdout);

    while (!terminate) {
        pause(); /* sleep until any handled signal arrives */

        if (toggle_pause) {
            toggle_pause = 0;
            if (running) {
                frozen += now_sec() - start;
                running = 0;
                printf("paused\n");
            } else {
                start = now_sec();
                running = 1;
                printf("resumed\n");
            }
            fflush(stdout);
        }

        if (print_elapsed) {
            print_elapsed = 0;
            printf("elapsed: %.3f s\n", elapsed_sec(running, start, frozen));
            fflush(stdout);
        }

        if (reset_timer) {
            reset_timer = 0;
            frozen = 0.0;
            start = now_sec();
            running = 1;
            printf("reset\n");
            fflush(stdout);
        }
    }

    printf("terminated cleanly (elapsed: %.3f s)\n",
           elapsed_sec(running, start, frozen));
    return 0;
}
