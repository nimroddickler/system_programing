#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define TIME_LIMIT 5

static volatile sig_atomic_t seconds;
static volatile sig_atomic_t timed_out;

void on_alarm(int sig) {
    int saved = errno;
    (void)sig;
    seconds++;
    write(STDOUT_FILENO, "\a", 1); /* beep */
    write(STDOUT_FILENO, "beep\n", 5);

    if (seconds >= TIME_LIMIT) {
        timed_out = 1;
        /* no alarm(1) — stop beeping */
    } else {
        alarm(1);
    }
    errno = saved;
}

int main(void) {
    const char *questions[] = {
        "Is the sky blue?",
        "Is 2 + 2 = 4?",
        "Do you like C?",
    };
    int num_of_questions = 3;
    char answer[4]; // "y" or "n" + \0

    struct sigaction sa;
    sa.sa_handler = on_alarm;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    if (sigaction(SIGALRM, &sa, NULL) != 0) {
        perror("sigaction");
        exit(EXIT_FAILURE);
    }

    int pid = getpid();
    printf("%d\n", pid);

    for (int i = 0; i < num_of_questions; i++) {
        printf("%s (y/n): ", questions[i]);
        fflush(stdout);

        seconds = 0;
        timed_out = 0;
        alarm(1);
        while (fgets(answer, sizeof(answer), stdin) == NULL) {
            if (timed_out)
                break;
            if (errno != EINTR)
                break;
        }
        alarm(0);

        printf("  (waited %d sec)\n", (int)seconds);

        if (timed_out)
            printf("  -> timed out\n");
        else if (answer[0] == 'y')
            printf("  -> yes\n");
        else if (answer[0] == 'n')
            printf("  -> no\n");
        else
            printf("  -> invalid\n");
    }

    return 0;
}
