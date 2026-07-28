#ifndef LINEBUF_H
#define LINEBUF_H
#include <stddef.h>

typedef struct {
    char   buf[1024];
    size_t len;      /* いま溜まっているバイト数 */
} linebuf_t;

/**
 * line_cb_t - 行が完成したときに呼ばれるコールバック関数の型
 * 取り出しと実際に何をするか、を分離させる。
 */
typedef void (*line_cb_t)(const char *line, void *ctx);

/**
 * fd から読めるだけ読み、完成した行ごとに cb を呼ぶ。
 * 戻り値:  1 = 継続, 0 = 相手が切断, -1 = エラー
 *
 * C にはクロージャがなく、コールバック関数は、呼ばれた瞬間に「呼び出し元の変数」を参照できない。
 * そのため、呼び出し側が渡したポインタを返すようにする。
 */
int linebuf_feed(linebuf_t *lb, int fd, line_cb_t cb, void *ctx);

#endif
