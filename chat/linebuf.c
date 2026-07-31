#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "linebuf.h"

/**
 * linebuf_feed
 *
 * 1. read(fd, buf + len, 残り容量)
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
    /* 空き容量が0の時は -1 を返す */
    if (sizeof lb->buf - lb->len == 0) {
        return -1;
    }

    /**
     * recv(fd, 受信したデータを格納するメモリ領域（バッファ）へのポインタ, バッファの残りサイズ)
     * read: ソケット以外も対象にできる
     *
     */
    ssize_t n = read(fd, lb->buf + lb->len, sizeof lb->buf - lb->len);
    fprintf(stderr, "[read] %zd bytes\n", n);

    if (n < 0) {
        perror("read");
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

    char *p;
    while ((p = memchr(lb->buf, '\n', lb->len)) != NULL) {

        /**
         * Windows では \r\n が改行として扱われるため、\r を \0 に置き換える
         */
        if (p > lb->buf && p[-1] == '\r') {
            p[-1] = '\0';
        }
        *p = '\0';
        cb(lb->buf, ctx);

        /* 最初の区切り文字までが消費量 */
        size_t consumed = (p - lb->buf) + 1;
        memmove(lb->buf, p + 1, lb->len - consumed);
        lb->len -= consumed;
    }
    return 1;
}
