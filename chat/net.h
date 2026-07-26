#ifndef NET_H
#define NET_H

/* 五目並べでもこのシグネチャをそのまま使う */
typedef enum { NET_LISTEN, NET_CONNECT, NET_RELAY } net_mode_t;

typedef struct {
    net_mode_t  mode;
    const char *host;      /* CONNECT のとき接続先 */
    const char *port;
    const char *room;      /* 中継サーバ用。今回は未使用 */
} net_config_t;

/* 接続済みの fd を返す。失敗なら -1 */
int net_connect(const net_config_t *cfg);

#endif
