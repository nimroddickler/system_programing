#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(int argc, char *argv[])
{
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <receiver-pid>\n", argv[0]);
        return 1;
    }
    int receiver_pid = atoi(argv[1]);

    for (;;) {
        char line[64];
        int count, offset;

        printf("> ");
        fflush(stdout);
        if (fgets(line, sizeof(line), stdin) == NULL)
            break;
        if (line[0] == 'e')
            break;

        sscanf(line, "%d %d", &count, &offset);
        if (offset < 0 || offset > 2) {
            fprintf(stderr, "Invalid offset: %d\n", offset);
            continue;
        }
        for (int i = 0; i < count; i++)
            kill(receiver_pid, SIGRTMIN + offset);
    }
    return 0;
}
