# ネットワーク学習ログ (2026-07-26)

`docs/CHAT_MINI_PROJECT.md` の Session 1 と Session 2 前半（実験）を実施した記録。

所要: 約3時間（09:18 - 12:14）。予定は Session 1 が50分だったが実質2時間20分かかった。
差分の大半は C の書き方を思い出す時間で、ネットワーク固有の内容ではない。

---

## 到達点

| 到達目標 | 状態 |
|---|---|
| 1. ソケット接続の確立手順を、サーバ側とクライアント側の違いを含めて説明できる | ✅ |
| 2. TCP がバイトストリームであり、メッセージ境界を保存しないことを再現できる | ✅ |
| 3. 受信バイト列から行を切り出すバッファを実装できる | 未（次回） |
| 4. `poll` で複数の入力を同時に待つ理由と書き方を説明できる | 未（次回） |

成果物: `chat/net.c`（接続確立）、`chat/main.c`（挨拶の送受信 + 実験コード）

---

# 1. 学習した内容

## 1.1 ソケットは fd という「番号」

開いているファイル・パイプ・ソケットは全て `int` の番号（fd）で表される。

```
0 = 標準入力 / 1 = 標準出力 / 2 = 標準エラー / 3以降 = 自分で開いたもの
```

**規約: 負なら失敗、0以上は成功。** `fd > 0` は誤り。標準入力を閉じた状態では `socket()` が 0 を返す（実測確認済み）。

```console
$ ./chat --host 8080 0<&-     # 標準入力を閉じて起動すると socket() が 0 を返しうる
```

ソケットもファイルも同じ fd なので `read` / `write` / `close` が同じように使える。Session 3 で `poll` に標準入力とソケットを並べて渡せるのはこのため。

## 1.2 名前解決は 1 対多

`getaddrinfo` は「候補のリスト」を返す。実測:

```
getaddrinfo("127.0.0.1", "8080")  → 候補1: 127.0.0.1 (IPv4)           … 1個
getaddrinfo("localhost",  "8080")  → 候補1: 127.0.0.1, 候補2: ::1      … 2個
getaddrinfo(NULL, "8080", AI_PASSIVE) → 候補1: ::, 候補2: 0.0.0.0      … 2個
```

相手が IPv4 でしか待っていなければ IPv6 の候補は失敗する。**順に試して最初に成功したものを使う。**

実際、使用中のポートに bind したとき `bind: Address already in use` が2回出た。候補を2つとも試している証拠。

## 1.3 アドレスは文字列ではなく構造体、しかも可変長

```
IPv4: ai_addrlen = 16
IPv6: ai_addrlen = 28
```

長さが違うので、`connect` / `bind` には**アドレスと長さの2つ**を渡す。文字列 → バイナリの変換は `getaddrinfo` が済ませており、結果は各候補の `ai_addr` に入っている。

## 1.4 サーバは fd を 2 つ持つ

```
サーバ:      socket → bind → listen → accept
クライアント: socket → connect
```

| fd | 正体 | できること |
|---|---|---|
| `listen_fd` | 窓口 | 接続を受け付ける。**送受信できない** |
| `accept` の戻り値 | 通話路 | send / recv できる |

`accept` は窓口を消費しない。呼ぶたびに別の通話路が生まれる（複数クライアントを捌ける）。待っている相手がいなければ `accept` はそこで待つ。

**窓口に `send` すると `Socket is not connected`**（実測）。このエラーが出たら fd を取り違えている。

## 1.5 3-way handshake はカーネルがやる

`accept()` は handshake を行わない。カーネルが SYN/SYN-ACK/ACK を完了させて待ち行列に積み、`accept` は**行列から1本取り出すだけ**。

確認方法: `listen()` の直後に `sleep(10)` を入れても、クライアントの `connect` は即座に成功する。

## 1.6 TCP はバイトストリーム ← 最重要

実験で確認した結果:

| | 送った側 | 受けた側 | 中身 |
|---|---|---|---|
| 実験1 | `send` を16回（1バイトずつ、100ms間隔） | `recv` が **16回**（各1バイト） | **1行**（1メッセージ） |
| 実験2 | `send` を **1回**（12バイト） | `recv` が **1回**（12バイト） | **2行**（2メッセージ） |

> **`recv` の回数とメッセージの個数には何の関係もない。**

1つのメッセージが16回に分割され、2つのメッセージが1回にまとまった。**両方向に間違っている。**

「1回 `send` したら相手も1回 `recv` で受け取れる」は **UDP の性質**。TCP が保証するのは「送った順序でバイトが届く」ことだけ。

### 実験2 で実際に起きたこと

```
[recv] 12 bytes: hello
world
peer closed
```

`hello` も `world` も表示されている。**何も消えていない。** 問題は**区切りが分からなくなったこと**。

五目並べで言えば、`"MOVE 5,5\nMOVE 6,6\n"` が1回で届いたとき、「1回の recv = 1コマンド」と書いたコードは全体を1手として解釈しようとして失敗するか、2手目を捨てる。**負荷が低いときは再現しないので、実戦で初めて壊れる。**

だから `\n` を目印に行を切り出す `linebuf` が要る。

---

# 2. 特に詰まった内容

症状から引けるように並べた。

## 2.1 `getaddrinfo` を自分で定義しようとした

```
error: static declaration of 'getaddrinfo' follows non-static declaration
netdb.h:271: note: previous declaration is here
```

チェックリストに書いてある関数名を見て「これを実装するのか」と誤解した。**`getaddrinfo` / `socket` / `bind` / `listen` / `accept` は全部システムの関数**で、呼ぶだけ。

**見分け方**: `man getaddrinfo` でマニュアルが出れば呼ぶだけ。出なければ自分で書く。

## 2.2 `bind` に文字列を渡した

```c
bind(fd, host, sizeof(host))    /* ✗ */
bind(fd, p->ai_addr, p->ai_addrlen)  /* ○ */
```

アドレスは文字列ではなく構造体（1.3）。そして**候補 `p` から取る**。`p` を `socket()` にしか使っていないと、せっかくのループが意味を成さない。

`sizeof(host)` は `const char *` のサイズなので**常に 8**。文字列長でもアドレス長でもない。

## 2.3 `struct addrinfo` と `struct sockaddr` の混同

```c
struct addrinfo client_addr;
accept(fd, (struct sockaddr*)&client_addr, &len);   /* ✗ */
```

| | 何者か |
|---|---|
| `struct addrinfo` | 候補の**情報カード**（family, addrlen, `ai_addr`, `ai_next`…） |
| `struct sockaddr` | **アドレスそのもの**（IPとポートのバイト列） |

`addrinfo` は `sockaddr` を**指している**入れ物。キャストは「型が違うことをコンパイラに黙らせる」だけで、**中身は変わらない**。

相手のアドレスが不要なら `accept(fd, NULL, NULL)` でよい。

## 2.4 `close(fd)` の直後に `recv(fd, ...)`

```c
close(fd);                    /* 窓口を閉じた */
recv(fd, buf, sizeof buf, 0); /* ✗ 閉じた窓口から読もうとしている */
```

通信に使うのは `accept` が返した通話路（1.4）。`close` を正しく足した結果、直後の取り違えが見えやすくなった形。

## 2.5 `NULL` をフラグに渡した

```c
recv(fd, buf, sizeof(buf), NULL)  /* ✗ NULL は (void*)0 = ポインタ */
recv(fd, buf, sizeof(buf), 0)     /* ○ フラグは int */
```

**`NULL` はポインタ用、`0` は整数用。** どちらもゼロだが型が違う。

## 2.6 `printf(stdout, "%s\n", n)` — 1行に3つの誤り

```c
printf(stdout, "%s\n", n);   /* ✗ */
printf("%s\n", buf);         /* ○ */
```

1. `printf` は出力先を取らない（それは `fprintf`）。`stdout` が書式文字列として渡っていた
2. `%s` に整数を渡すと**その数値をアドレスとみなして読む** → セグフォ（実測: exit 139）
3. 表示すべきは長さ `n` ではなく中身 `buf`

## 2.7 `char msg[] = 条件 ? A : B`

```
error: array initializer must be an initializer list or string literal
```

配列の初期化子に書けるのは文字列リテラルか `{...}` だけ。条件で切り替えたいなら**ポインタ**にする。

```c
const char *msg = cond ? "a\n" : "b\n";   /* ○ */
```

| | `char msg[]` | `const char *msg` |
|---|---|---|
| `sizeof` | 配列のバイト数（17） | **常に 8** |
| 書き換え | できる | できない |
| 条件で切替 | できない | できる |

## 2.8 `send(fd, &msg, ...)` — `&` の要否

ASan が捕捉:

```
READ of size 12 at ...
  [96, 104) 'msg' (line 46) <== Memory access at offset 104 overflows this variable
```

**8バイトの変数から12バイト読もうとしている** = `msg` はポインタなのに `&` を付けた。

**判断基準:**

> 変数が**データそのもの**を持っている → `&` が要る（`&hints`, `&res`）
> 変数が**すでにアドレス**を持っている → `&` は不要（`msg`）

## 2.9 `perror` を errno 未設定で呼んだ

```
connection closed
: Undefined error: 0
```

`perror` は `errno` を読んで理由を付け足す関数。**システムコールが失敗（-1）したときだけ**使う。`recv` が `0` を返すのは正常な切断なので `errno` は設定されていない。

また `perror("msg\n")` のように**改行を入れない**。理由が次の行に落ちる。

## 2.10 `if` の波括弧が無く `break` が常に実行された

```c
if (n == 0)
    fprintf(...);
    break;        /* ✗ if の外。毎回実行される */
```

```
warning: misleading indentation; statement is not part of the previous 'if'
```

**波括弧の無い `if` は直後の1文だけを支配する。** C は改行とインデントを無視するので、人間が読む構造とコンパイラが読む構造がずれる。

予防: **`if` の中身が1行でも常に `{}` を書く。**

## 2.11 `while (0)` / `int i;`

- `while (0)` は常に偽。ループが1度も回らない
- `for (size_t i; ...)` は**初期化されていない**。C は自動で 0 にしない。コンパイラも検出しないことがある

**宣言したら初期化する。**

## 2.12 出力順序が狂う（2回踏んだ）

```
peer closed. [recv] 0 bytes:      ← 最後に起きたのに先頭に出る
peer connected
h
e
...
```

| 出力先 | 挙動 |
|---|---|
| `stderr` | **バッファしない**。呼んだ瞬間に出る |
| `stdout` | ファイル/パイプへ出すときは**溜めてから**まとめて出す |

ターミナルに直接出すと行ごとに出るので気づかない。**リダイレクトした瞬間に順序が狂う。**

対策: 時系列が重要なログは `stderr` に出す。`stdout` なら要所で `fflush(stdout)`。

---

# 3. 覚えておく要点

## 3.1 後片付け

C には自動の後始末が無い。**「この経路を通ったとき、さっき確保したものはどうなる?」を毎回自問する。**

| 確保したもの | 返す関数 |
|---|---|
| `socket()` の fd | `close()` |
| `getaddrinfo()` の候補リスト | `freeaddrinfo()` |

踏んだ具体例:
- `setsockopt` 失敗時に `close(fd)` を忘れた（fd リーク）
- `close(fd)` した後 `fd = -1` に戻し忘れると、**閉じた fd を有効と誤判定**して `listen` が `Bad file descriptor` になる
- 失敗経路で `freeaddrinfo` を通らない（メモリリーク）。**成功経路だけ見ていると気づかない**

macOS の ASan は**リーク検出に対応していない**（`detect_leaks is not supported on this platform`）。Linux の CI なら検出できる。

## 3.2 両側が `recv` で待つと止まる

同じコードが両側で動くので、順序を間違えると両方が固まる。

| 書き方 | 結果 |
|---|---|
| 両方が `send` → `recv` | ✅ 動く |
| 両方が `recv` → `send` | ❌ **デッドロック** |
| 両方が「相手が切るまで recv ループ」 | ❌ **デッドロック** |

実験では役割を非対称にして回避した（送信役は recv しない）。**根本的な解決が Session 3 の `poll`。**

## 3.3 `nc` で切り分ける

自作の両側でデバッグすると、どちらが悪いか分からない。**片方を `nc` に置き換える。**

| やりたいこと | コマンド |
|---|---|
| サーバ側だけ検証 | `nc 127.0.0.1 8080` |
| クライアント側だけ検証 | `nc -l 8080` |
| 送信内容を自分で決める | `echo "任意" \| nc 127.0.0.1 8080` |

**`nc` は自動では何も送らない。** 打った文字だけを送る。「相手の挨拶が出ない」のは当然。

## 3.4 ASan レポートは3箇所だけ読む

| 見る場所 | 分かること |
|---|---|
| `READ/WRITE of size N` | 何バイト触ろうとしたか |
| `#N in main main.c:58` | どの行か |
| `[開始, 終了) '変数名'` | その変数の**実際のサイズ** |

「8バイトの変数から12バイト読んでいる」と分かれば原因は絞れる。

## 3.5 同じPCの別ターミナルで完結する

`127.0.0.1` はループバック。データは**LANにもWi-Fiにも出ず**、OSの中で折り返す。

ポート番号は「1台の中でどのプログラム宛か」を区別する番号。だから同じポートを2つのプログラムが同時に使えず、`bind` が `Address already in use` で失敗する。

2台でやるときは `127.0.0.1` を相手のLAN内アドレス（`ipconfig getifaddr en0`）に変えるだけ。**コードの変更は不要。**

## 3.6 macOS 固有

| 一般的な記事の記述 | macOS |
|---|---|
| `ss -tan` | 存在しない。`netstat -an \| grep PORT` または `lsof -nP -i :PORT` |
| `tcpdump -i lo` | `lo0` |
| ASan のリーク検出 | 非対応 |

`-std=c11` で POSIX 関数（`getaddrinfo` / `poll` / `usleep`）が使えるのは macOS だから。**Linux (glibc) では隠れる**ので `-std=gnu11` か `-D_POSIX_C_SOURCE=200809L` が要る。

---

# 4. 次回やること

## Session 2 後半: `linebuf` の実装（60分）

受信バイト列から `\n` 区切りで行を切り出す。

```c
typedef struct {
    char   buf[1024];
    size_t len;
} linebuf_t;

int linebuf_feed(linebuf_t *lb, int fd, line_cb_t cb, void *ctx);
```

満たすべき要件（今日の実験がそのまま要件になっている）:

- 実験1のように**1バイトずつ届いても**、行が揃うまで溜めて1行として渡す
- 実験2のように**2行まとめて届いても**、2回に分けてコールバックを呼ぶ
- `\n` を含まない残りは**捨てずに次の `recv` と繋げる** ← ここが本質
- バッファが満杯なのに `\n` が無ければ切断（無制限に伸ばすと DoS になる）

**検証方法**: `chat/main.c` にコメントで残した実験1の1バイト送信コードを有効にして動かす。正しく行が組み立てられれば成功。**今日の実験がそのままテストになる。**

注意点:
- `memcpy` ではなく `memmove`（領域が重なる）
- `recv` の第3引数は `sizeof lb->buf - lb->len`（残り容量。`sizeof lb->buf` だと溢れる）
- `recv` の結果は NUL 終端されていない。長さを持ち回る

## Session 3: `poll`（40分）

3.2 のデッドロックを根本的に解く。標準入力とソケットを並べて監視する。

**既知の落とし穴**（`docs/CHAT_MINI_PROJECT.md` に記載）: macOS では `POLLIN` と `POLLHUP` が同時に立つ。`POLLHUP` を先に見て `break` すると**最後の行を取りこぼす**。`POLLIN` を先に処理すること。

## そのあと: 五目並べへ

| 成果物 | 移植 |
|---|---|
| `linebuf.c` / `.h` | そのまま100% |
| `net.c` / `.h` | `net_connect` の中身がそのまま |
| `poll` ループ | 骨組みがそのまま |

順序: [#30](https://github.com/kazu-hide/TicTacToe_w_C/issues/30)（ゲームコアと I/O の分離）→ [#31](https://github.com/kazu-hide/TicTacToe_w_C/issues/31)（不正手の理由を返す）→ [#40](https://github.com/kazu-hide/TicTacToe_w_C/issues/40)（シリアライズ）→ [#39](https://github.com/kazu-hide/TicTacToe_w_C/issues/39)（プロトコル設計）
