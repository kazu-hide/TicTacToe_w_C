#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include "net.h"
# include <unistd.h>

int main(int argc, char **argv) {
    net_config_t cfg = {0};

    if (argc == 3 && strcmp(argv[1], "--host") == 0) {
        cfg.mode = NET_LISTEN;
        cfg.port = argv[2];
    } else if (argc == 3 && strcmp(argv[1], "--join") == 0) {
        /* "127.0.0.1:8080" を host と port に割る */
        static char buf[256];
        snprintf(buf, sizeof buf, "%s", argv[2]);
        char *colon = strrchr(buf, ':');
        if (!colon) { fprintf(stderr, "usage: %s --join HOST:PORT\n", argv[0]); return 1; }
        *colon = '\0';
        cfg.mode = NET_CONNECT;
        cfg.host = buf;
        cfg.port = colon + 1;
    } else {
        fprintf(stderr, "usage: %s --host PORT | --join HOST:PORT\n", argv[0]);
        return 1;
    }

    int fd = net_connect(&cfg);
    if (fd < 0) return 1;

    const char *msg = cfg.mode == NET_CONNECT ? "hello from join\n" : "hello from host\n";

    if(send(fd, msg, strlen(msg), 0) < 0)
        perror("send");

    char buf[64];
    ssize_t n = recv(fd, buf, sizeof(buf) - 1, 0);
    if (n < 0)
        perror("recv");
    if (n == 0)
        fprintf(stderr, "connection closed\n");
    if (n > 0) {
        buf[n] = '\0'; /* recvは終端文字がないため追加 */
        printf("%s\n", buf);
    }

    close(fd);
    return 0;
}
