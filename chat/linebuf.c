#include <stdio.h>
#include <string.h>
#include <sys/socket.h>

#include "linebuf.h"

/**
 * linebuf_feed
 *
 * 1. recv(fd, buf + len, 残り容量)
 *  戻り値 > 0 → len += n、続行
 *  戻り値 = 0 → return 0（相手が切断）
 *  戻り値 < 0 → return -1（エラー）
 * 2.  while (buf の中に \n がある)
 *      a. \n の位置を p とする
 *      b. p の1つ手前が \r ならそこも '\0'
 *      c. *p = '\0'
 *      d. cb(buf, ctx)
 *      e. 消費量 = (p - buf) + 1
 *      f. memmove(buf, p + 1, len - 消費量)
 *      g. len -= 消費量
 * 3. \n が無いまま満杯なら return -1
 * 4. return 1 (継続)
 */
int linebuf_feed(linebuf_t *lb, int fd, line_cb_t cb, void *ctx)
{
    /**
     * recv(fd, 受信したデータを格納するメモリ領域（バッファ）へのポインタ, バッファの残りサイズ)
     *
     */
    ssize_t n = recv(fd, lb->buf + lb->len, sizeof lb->buf - lb->len, 0);

    if (n < 0) {
        perror("recv");
        return -1;
    }
    if (n == 0) {
        return 0;
    }

    lb->len += n;

    /*
     * memchr で \n を探す
     * memchr(buf, '\n', len)
     * 探索開始位置、探索char, 探索範囲
     */
    char *p = memchr(lb->buf, '\n', lb->len);
    if (p != NULL) {
        *p = '\0';
        cb(lb->buf, ctx);
        lb->len = 0;
    }
    return 1;
}
