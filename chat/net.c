/*
 * net.c — Session 1 の実装対象。
 *
 */
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netdb.h>

#include "net.h"

/* --- サーバ側 ------------------------------------------------------------
 * 1. getaddrinfo(NULL, port, &hints, &res)
 *      hints.ai_family   = AF_UNSPEC
 *      hints.ai_socktype = SOCK_STREAM
 *      hints.ai_flags    = AI_PASSIVE      <- 第1引数 NULL とセット
 * 2. res を先頭から順に試す:
 *      socket() -> setsockopt(SO_REUSEADDR) -> bind()
 *      成功したら抜ける。失敗したら close() して次の候補へ
 * 3. freeaddrinfo(res)
 * 4. listen()
 * 5. accept()   <- 戻り値は「別の fd」。リスニング fd を上書きしないこと
 * 6. リスニング fd はもう使わないので close() してよい（今回は1対1なので）
 *
 * 失敗したら perror("...") で理由を出してから -1 を返す
 */
static int net_listen_and_accept(const char *port)
{
    /* getaddrinfo に「どんな候補が欲しいか」を伝えるための構造体。
     * 使わないフィールドは 0 でなければならないので memset で全部潰す。
     * (sizeof hints は「hints 変数のバイト数」。括弧は要らない) */
    struct addrinfo hints;
    memset(&hints, 0, sizeof hints);
    hints.ai_family   = AF_UNSPEC;    /* IPv4 でも IPv6 でもよい */
    hints.ai_socktype = SOCK_STREAM;  /* TCP */
    hints.ai_flags = AI_PASSIVE;

    /* addrinfoは複数の値を返そうとするが、C言語関数は一つの値しか返せない。
     * そのため、ポインタを渡し結果を書き込んでもらう。*/
    struct addrinfo *res = NULL;
    int rc = getaddrinfo(NULL, port, &hints, &res);
    if (rc != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rc));
        return -1;
    }

    /* resは連結リスト。 */
    int fd = -1; /* file discriptor 初期化 */
    for (struct addrinfo *p = res; p != NULL; p = p->ai_next) {
        fd = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
        if (fd < 0)
            continue;

        /*
         * あまり間隔を空けずにserverを起動しようとするとアドレスがまだ使用中となる。
         * 理由はTCPプロトコルの仕様による。
         * TCPプロトコルは接続終了後、そのソケットをTIME_WAIT状態にし、その間はまだ利用できない。
         * その設定を変更して、TIME_WAITでもアドレスを利用できるようにする。
         */
        int optval = 1;
        if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval)) < 0) {
          perror("setsockopt");
          close(fd);
          fd = -1;
          continue;
        }

        if (bind(fd, p->ai_addr, p->ai_addrlen) == 0)
            break;

        perror("bind");
        close(fd);
        fd = -1;
    }
    freeaddrinfo(res);

    if (fd < 0) {
        fprintf(stderr, "could not open server with %s\n", port);
        return -1;
    }

    if (listen(fd, 5) < 0) {
        perror("listen");
        close(fd);
        return -1;
    }

    int sock = accept(fd, NULL, NULL);
    if (sock < 0) {
      perror("accept");
      close(fd);
      return -1;
    }

    printf("peer connected\n");
    close(fd);
    return sock;
}

/* --- クライアント側 ------------------------------------------------------
 * 1. getaddrinfo(host, port, &hints, &res)
 *      hints.ai_family   = AF_UNSPEC
 *      hints.ai_socktype = SOCK_STREAM
 *      （AI_PASSIVE は付けない）
 * 2. res を先頭から順に試す:
 *      socket() -> connect()
 *      成功したら抜ける。失敗したら close() して次の候補へ
 * 3. freeaddrinfo(res)
 *
 * 失敗したら perror("...") で理由を出してから -1 を返す
 */
static int net_connect_to(const char *host, const char *port)
{
    /* getaddrinfo に「どんな候補が欲しいか」を伝えるための構造体。
     * 使わないフィールドは 0 でなければならないので memset で全部潰す。
     * (sizeof hints は「hints 変数のバイト数」。括弧は要らない) */
    struct addrinfo hints;
    memset(&hints, 0, sizeof hints);
    hints.ai_family   = AF_UNSPEC;    /* IPv4 でも IPv6 でもよい */
    hints.ai_socktype = SOCK_STREAM;  /* TCP */

    /* res は getaddrinfo が結果を書き込むための「出力用ポインタ」。
     * 関数に値を返させたいので、ポインタのアドレス (&res) を渡す。 */
    struct addrinfo *res = NULL;
    int rc = getaddrinfo(host, port, &hints, &res);
    if (rc != 0) {
        /* getaddrinfo は errno を使わないので perror ではなく gai_strerror */
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rc));
        return -1;
    }

    /* res は連結リスト。p->ai_next を辿って候補を順に試す。
     * 「最初に成功したものを使う」のが正しい作法。 */
    int fd = -1;
    for (struct addrinfo *p = res; p != NULL; p = p->ai_next) {
        fd = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
        if (fd < 0)
            continue;                 /* この候補ではソケットを作れない。次へ */

        if (connect(fd, p->ai_addr, p->ai_addrlen) == 0)
            break;                    /* 成功。この fd を使う */

        perror("connect");
        close(fd);                    /* 失敗した fd は捨てる (閉じ忘れは資源リーク) */
        fd = -1;
    }

    freeaddrinfo(res);                /* getaddrinfo が確保したメモリを返す */

    if (fd < 0) {
        fprintf(stderr, "could not connect to %s:%s\n", host, port);
        return -1;
    }

    printf("connected\n");
    return fd;
}

int net_connect(const net_config_t *cfg)
{
    switch (cfg->mode) {
    case NET_LISTEN:
        return net_listen_and_accept(cfg->port);
    case NET_CONNECT:
        return net_connect_to(cfg->host, cfg->port);
    case NET_RELAY:
        fprintf(stderr, "NET_RELAY is not supported yet\n");
        return -1;
    }
    return -1;
}
