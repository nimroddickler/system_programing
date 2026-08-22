#include <signal.h>
#include <stdio.h>
#include <unistd.h>

static volatile sig_atomic_t counter;
static volatile sig_atomic_t got_print;

static void on_signal(int sig)
{
    if (sig == SIGRTMIN) {
        counter++;
    } else if (sig == SIGRTMIN + 1) {
        counter--;
    } else if (sig == SIGRTMIN + 2) {
        got_print = 1;
    }
}

int main(void)
{
    int pid = getpid();
    printf("%d\n", pid);

    struct sigaction sa;
    sa.sa_handler = on_signal;
    sa.sa_flags = 0;
    sigemptyset(&sa.sa_mask);

    sigaction(SIGRTMIN, &sa, NULL);
    sigaction(SIGRTMIN + 1, &sa, NULL);
    sigaction(SIGRTMIN + 2, &sa, NULL);

    for (;;) {
        pause();
        if (got_print) {
            printf("%d\n", counter);
            got_print = 0;
        }
    }
}
