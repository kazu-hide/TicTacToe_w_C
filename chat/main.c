#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <poll.h>
#include <errno.h>
#include <signal.h> 
#include "net.h"
#include "linebuf.h"

static int send_all(int fd, const char *msg)
{
    size_t len = strlen(msg);
    size_t sent = 0;
    ssize_t n = 0;

    while (sent < len) {
        n = send(fd, msg + sent, len - sent, 0);
        if (n < 0 && errno == EINTR) {
            continue;
        }
        if (n < 0) {
            perror("send");
            return -1;
        }
        sent += n;
    }
    return 0;
}


static void on_recv(const char *line, void *ctx)
{
    (void)ctx;
    fprintf(stderr, "[recv] %s\n", line);
}

typedef struct {
    int fd;
    int send_failed;
} send_ctx_t;

static void on_send(const char *line, void *ctx)
{
    send_ctx_t *c = ctx;

    if (c->send_failed) {
        return;
    }

    /* 1024 + '\n' + '\0' (送る際には 改行+終端 を追加) */
    char out[1026];
    snprintf(out, sizeof out, "%s\n", line);

    if (send_all(c->fd, out) < 0) {
        c->send_failed = 1;
        return;
    }
    fprintf(stderr, "[sent] %s\n", line);
}

/**
*  [キーボード/パイプ] ──→ カーネル ──read──→ stdin_lb ──cb──→ send  ──→ 相手
*  [相手]              ──→ カーネル ──read──→ sock_lb  ──cb──→ 画面
* 
*/
static int chat_loop(int fd)
{
    int status = 0;

    linebuf_t sock_lb = {0};
    linebuf_t stdin_lb = {0};

    send_ctx_t sctx = {
        .fd = fd,
        .send_failed = 0
    };
    
    struct pollfd pfds[2] = {
        { .fd = STDIN_FILENO, .events = POLLIN }, // stdin
        { .fd = fd,      .events = POLLIN }       // network socket
    };    

    int n;

    for (;;) {
       if((n=poll(pfds, 2, 1000)) < 0){
            if (errno == EINTR) {
                continue;
            } else {
                status = 1;
                perror("poll");
                break;        
            }
        }
        if (n == 0) {
            continue;
        }

        /**
        * ソケットが先、標準入力が後。POLLHUP が理由。
        *
        * POLLHUP は「もう新しくは来ない」であって「もう無い」ではないので、
        * POLLIN と同じ扱いにして read を試みる。既に届いている分を読み切る
        * まで抜けてはいけない。読み切ったかどうかは read が 0 を返したか
        * (linebuf_feed の rc == 0) だけが教えてくれる。
        *
        * POLLERR / POLLNVAL は別扱い。POLLNVAL は fd 自体が無効で POLLIN が
        * 立たないため、処理しないと poll が即座に返り続けて暴走する。
        */
        /* ソケットのデータは受信する */
        if (pfds[1].revents & (POLLIN | POLLHUP)) {
            int rc = linebuf_feed(&sock_lb, fd, on_recv, NULL);
            if (rc < 0) {
                status = 1;
                fprintf(stderr, "linebuf_feed failed\n");
                break;
            }
            if (rc == 0) {
                fprintf(stderr, "peer closed\n");
                break;
            }
            /* rc == 1 なら継続 */
        }
        if (pfds[1].revents & (POLLERR | POLLNVAL)) {
            status = 1;
            break;
        }


        /* 標準入力のデータは送信する */
        if (pfds[0].revents & (POLLIN | POLLHUP)) {
            int rc = linebuf_feed(&stdin_lb, STDIN_FILENO, on_send, &sctx);
            if (rc < 0) {
                status = 1;
                fprintf(stderr, "linebuf_feed failed\n");
                break;
            }
            if (rc == 0) {
                fprintf(stderr, "stdin closed\n");
                break;
            }
            if (sctx.send_failed) {
                status = 1;
                fprintf(stderr, "send failed\n");
                break;
            }
            /* rc == 1 なら継続 */
        }
        if (pfds[0].revents & (POLLERR | POLLNVAL)) {
            status = 1;
            break;
        }
    }
    return status;
}

int main(int argc, char **argv) {
    signal(SIGPIPE, SIG_IGN);

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

    /* 通常モード: 両側が挨拶を送り合う */
    int status = chat_loop(fd);
    close(fd);
    
    return status;
}
