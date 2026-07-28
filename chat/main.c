#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include "net.h"
#include "linebuf.h"

/* ------------------------------------------------------------------
 * Session 2 の実験用スイッチ。両方 0 にすると Session 1 の挙動に戻る。
 *
 *   実験1: SLOW_SEND=1  "hello\nworld\n" を1バイトずつ 100ms 間隔で送る
 *          → 1つのメッセージが複数回の recv に分割されることを見る
 *   実験2: TWO_LINES=1  "hello\nworld\n" を1回の send で送る
 *          → 2つのメッセージが1回の recv にまとまることを見る
 *
 * 実験中は送信役(--join)と受信役(--host)を非対称にする。理由は2つ。
 *   - 両側が recv でループすると互いに待ち合ってデッドロックする
 *   - 受信役が送ると、相手が読まないまま切断して RST が飛ぶ
 *     (Connection reset by peer になり観測が汚れる)
 * ------------------------------------------------------------------ */
#define EXPERIMENT_SLOW_SEND 0
#define EXPERIMENT_TWO_LINES 1

#define EXPERIMENT (EXPERIMENT_SLOW_SEND || EXPERIMENT_TWO_LINES)

/* 1回で送る */
static void send_at_once(int fd, const char *msg)
{
    if (send(fd, msg, strlen(msg), 0) < 0)
        perror("send");
}

/* 1バイトずつ間隔を空けて送る (実験1)。
 * 間隔を空けないと OS がまとめてしまい、分割が観測できない */
static void send_slowly(int fd, const char *msg)
{
    size_t len = strlen(msg);

    for (size_t i = 0; i < len; ++i) {
        if (send(fd, msg + i, 1, 0) < 0) {
            perror("send");
            return;
        }
        usleep(100000);
    }
}

static void on_line(const char *line, void *ctx)
{
    (void)ctx;
    fprintf(stderr, "[line] %s\n", line);
}

/**
 * 相手が切断するまで linebuf_feed を繰り返し、区切り文字ごとに callback(on_line) を呼び出す
 */
static void recv_lines(int fd)
{
    linebuf_t lb = {0};

    for (;;) {
        int rc = linebuf_feed(&lb, fd, on_line, NULL);
        if (rc < 0) {
            perror("linebuf_feed");
            break;
        }
        if (rc == 0) {
            fprintf(stderr, "peer closed\n");
            break;
        }
        /* rc == 1 なら継続 */
    }
}

/* Session 1 の挙動: 挨拶を送り、相手の挨拶を1回受け取る */
static void exchange_greeting(int fd, const net_config_t *cfg)
{
    const char *msg = (cfg->mode == NET_CONNECT) ? "hello from join\n"
                                                 : "hello from host\n";
    send_at_once(fd, msg);

    char buf[64];
    ssize_t n = recv(fd, buf, sizeof buf, 0);

    if (n < 0)
        perror("recv");
    else if (n == 0)
        fprintf(stderr, "peer closed\n");
    else
        printf("%.*s", (int)n, buf);  /* 相手の挨拶は既に \n で終わっている */
}

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

    if (EXPERIMENT) {
        /* 実験モード: --join が送るだけ、--host が受けるだけ */
        if (cfg.mode == NET_CONNECT) {
            const char *msg = "hello\nworld\n";

            if (EXPERIMENT_SLOW_SEND)
                send_slowly(fd, msg);
            else
                send_at_once(fd, msg);
        } else {
            recv_lines(fd);
        }
    } else {
        /* 通常モード: 両側が挨拶を送り合う */
        exchange_greeting(fd, &cfg);
    }

    close(fd);
    return 0;
}
