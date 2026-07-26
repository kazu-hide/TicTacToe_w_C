#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include "net.h"

/* ------------------------------------------------------------------
 * Session 2 の実験用スイッチ。両方 0 に戻すと Session 1 の挙動になる。
 *
 *   実験1: SLOW_SEND=1, TWO_LINES=0   1バイトずつ 100ms 間隔で送る
 *   実験2: SLOW_SEND=0, TWO_LINES=1   "hello\nworld\n" を1回の send で送る
 *
 * 実験中は送信役(--join)と受信役(--host)を非対称にしている。
 * 両側が recv でループすると互いに待ち合ってデッドロックするため。
 * ------------------------------------------------------------------ */
#define EXPERIMENT_SLOW_SEND 1
#define EXPERIMENT_TWO_LINES 0

#define EXPERIMENT (EXPERIMENT_SLOW_SEND || EXPERIMENT_TWO_LINES)

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

    //const char *msg = cfg.mode == NET_CONNECT ? "hello from join\n" : "hello from host\n";

    /**
     * 実験2
     * "hello\nworld\n"を1回でsendしたら何回 recvを受け取るか
     */
    const char *msg = "hello\nworld\n";

    if (cfg.mode == NET_CONNECT) {
        /*
         * 実験1にて 1 byte ごとに送るためのコード
         *
        for (size_t i = 0; i < strlen(msg); ++i) {
            if(send(fd, msg + i, 1, 0) < 0)
                perror("send");
            usleep(100000);
        }
         */
        if(send(fd, msg, strlen(msg), 0) < 0)
            perror("send");
    }

    if (cfg.mode == NET_LISTEN) {
        while (1) {
            char buf[64];
            ssize_t n = recv(fd, buf, sizeof(buf) - 1, 0);
            if (n < 0)
                perror("recv");
            if (n == 0) {
                fprintf(stderr, "peer closed\n");
                break;
            }
            if (n > 0) {
                buf[n] = '\0'; /* recvは終端文字がないため追加 */
                fprintf(stderr, "[recv] %zd bytes: %.*s\n", n, (int)n, buf);
            }
        }
    }

    close(fd);
    return 0;
}
