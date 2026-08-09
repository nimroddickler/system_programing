#include <stdio.h>
#include <unistd.h>
#include <signal.h>

volatile sig_atomic_t counter = 0;

void on_signal(int sig) {
    if (sig == SIGRTMIN) {
        counter++;
    } else if (sig == SIGRTMIN + 1) {
        counter--;
    } else if (sig == SIGRTMIN + 2) {
        printf("%d\n", counter);
    }
}

int main(void) {
    int pid = getpid();
    printf("%d\n", pid);

    struct sigaction sa;
    sa.sa_handler = on_signal;
    sa.sa_flags = 0;
    sigemptyset(&sa.sa_mask);
    
    sigaction(SIGRTMIN, &sa, NULL);
    sigaction(SIGRTMIN + 1, &sa, NULL);
    sigaction(SIGRTMIN + 2, &sa, NULL);

    for (;;)
        pause();    // loop forever waiting for signals

    return 0;
}
