/* Minimal TCP echo server — only the essential steps */
#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

int main(void) {
    /* Step 1: create a socket (IPv4 + TCP) */
    int s = socket(PF_INET, SOCK_STREAM, 0);

    /* Step 2: choose address = any local IP, port 9090 */
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr)); /* Initialize the address structure to 0 */
    addr.sin_family = AF_INET; /* Set the address family to IPv4 */
    addr.sin_addr.s_addr = htonl(INADDR_ANY);  /* 0.0.0.0 = all interfaces */
    addr.sin_port = htons(9090);

    /* Step 3: attach that address to our socket */
    bind(s, (struct sockaddr *)&addr, sizeof(addr));

    /* Step 4: start waiting for clients */
    listen(s, 1);
    printf("waiting on port 9090...\n");

    /* Step 5: accept one client (blocks until someone connects) */
    int c = accept(s, NULL, NULL);
    printf("client connected\n");

    /* Step 6: read what they send, write it back, until they leave */
    char buf[256];
    int n;
    while ((n = recv(c, buf, sizeof(buf), 0)) > 0) {
        send(c, buf, n, 0);   /* echo */
        fwrite(buf, 1, n, stdout);
    }

    close(c);
    close(s);
    return 0;
}
