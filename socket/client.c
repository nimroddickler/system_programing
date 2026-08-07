/* Minimal TCP client — connect, send one line, print the reply */
#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

int main(void) {
    /* Step 1: create a socket (IPv4 + TCP) */
    int s = socket(PF_INET, SOCK_STREAM, 0);

    /* Step 2: server address = 127.0.0.1:9090 (this machine) */
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(9090);
    inet_pton(AF_INET, "127.0.0.1", &addr.sin_addr);

    /* Step 3: connect to the server */
    connect(s, (struct sockaddr *)&addr, sizeof(addr));
    printf("connected — type a line:\n");

    /* Step 4: send what you type */
    char buf[256];
    fgets(buf, sizeof(buf), stdin);
    send(s, buf, strlen(buf), 0);

    /* Step 5: read the echo and print it */
    int n = recv(s, buf, sizeof(buf) - 1, 0);
    if (n > 0) {
        buf[n] = '\0';
        printf("server said: %s", buf);
    }

    close(s);
    return 0;
}
