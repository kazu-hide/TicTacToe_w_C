#!/bin/bash
#
# chat の結合テスト。chat/ で ./test.sh
#
# なぜパイプで検証するのか
#   端末から手で叩くと、入力は「1行ずつ・ゆっくり・EOF なし」でしか届かない。
#   下記のバグはどれも入力の「届き方」に依存していて、手動操作では 100% 再現
#   しない。パイプなら「まとめて・一瞬で・EOF 付き」を作れる。
#
#   - POLLIN と POLLHUP の同時発生 (revents = 0x11)
#     擬似端末は 0x01 しか返さないので、手打ちでは POLLHUP の分岐に入らない
#   - POLLHUP を読み切る前に break して取りこぼす
#   - send の失敗が呼び出し側に伝わらない
#
# 使い方
#   ./test.sh              8080 番で実行
#   PORT=9000 ./test.sh    ポートを変える
#
set -u
set +m          # pkill 時のジョブ終了通知を出さない

PORT=${PORT:-8080}
TMP=$(mktemp -d)
PASS=0
FAIL=0

cleanup() {
    pkill -f "chat --host $PORT"          2>/dev/null
    pkill -f "chat --join 127.0.0.1:$PORT" 2>/dev/null
    pkill -f "deadpeer-$PORT"              2>/dev/null
    rm -rf "$TMP"
}
trap cleanup EXIT

reset_procs() {
    pkill -f "chat --host $PORT"          2>/dev/null
    pkill -f "chat --join 127.0.0.1:$PORT" 2>/dev/null
    pkill -f "deadpeer-$PORT"              2>/dev/null
    sleep 0.5
}

check() {   # check <項目名> <期待値> <実際の値>
    if [ "$2" = "$3" ]; then
        printf '    PASS  %-22s %s\n' "$1" "$3"
        PASS=$((PASS + 1))
    else
        printf '    FAIL  %-22s 期待=%s  実際=%s\n' "$1" "$2" "$3"
        FAIL=$((FAIL + 1))
    fi
}

lines() {   # lines <n> : 0..n-1 を1行ずつ
    python3 -c "print('\n'.join(str(i) for i in range($1)))"
}

# 接続を受けた直後に閉じる相手役。プロセス名で kill できるようファイルにする
cat > "$TMP/deadpeer-$PORT.py" <<PYEOF
import socket, time
srv = socket.socket()
srv.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
srv.bind(('127.0.0.1', $PORT)); srv.listen(1)
c, _ = srv.accept(); c.close()
time.sleep(30)
PYEOF

echo "== ビルド =="
make || exit 1
echo

# ---------------------------------------------------------------- 1
# 500行を一度に流す。書き手 (python) は書き終わると即終了するので
# stdin の revents は 0x11 (POLLIN + POLLHUP) になる。
# POLLHUP を読み切らずに break すると 283 行で打ち切られる。
echo "1) 500行 / 書き手が即終了 (POLLHUP あり)"
reset_procs
( sleep 10 | ./chat --host $PORT > "$TMP/h1" 2>&1 ) &
sleep 1
lines 500 | ./chat --join 127.0.0.1:$PORT > "$TMP/j1" 2>&1
code=$?
sleep 0.5
check "exit"   0   "$code"
check "送信行数" 500 "$(grep -c '^\[sent\]' "$TMP/j1")"
check "受信行数" 500 "$(grep -c '^\[recv\]' "$TMP/h1")"
echo

# ---------------------------------------------------------------- 2
# 書き手を生かしたまま (sleep) にすると POLLHUP が立たない。
# 1 と同じ結果になることで、POLLHUP の有無で挙動が変わらないことを確認する。
echo "2) 500行 / 書き手が生存 (POLLHUP なし)"
reset_procs
( sleep 10 | ./chat --host $PORT > "$TMP/h2" 2>&1 ) &
sleep 1
( lines 500; sleep 5 ) | ./chat --join 127.0.0.1:$PORT > "$TMP/j2" 2>&1 &
sleep 4
check "送信行数" 500 "$(grep -c '^\[sent\]' "$TMP/j2")"
check "受信行数" 500 "$(grep -c '^\[recv\]' "$TMP/h2")"
echo

# ---------------------------------------------------------------- 3
# 既に閉じた相手に送り続ける。SIGPIPE を無視しているので即死はしない。
# send の失敗が on_send -> chat_loop -> main まで伝わることを確認する。
# 伝わっていないと Broken pipe が数百件出て exit 0 になる。
echo "3) 既に閉じた相手へ 500行"
reset_procs
python3 "$TMP/deadpeer-$PORT.py" 2>/dev/null &
sleep 1
lines 500 | ./chat --join 127.0.0.1:$PORT > "$TMP/j3" 2>&1
code=$?
# errno を決め打ちしない。RST 到着後、send は EPIPE (Broken pipe) を返し、
# recv は ECONNRESET (Connection reset by peer) を返す (操作によって違う)。
# プラットフォーム差もあるので、種類は問わず件数だけを見る。
check "exit"            1 "$code"
check "send エラー件数"  1 "$(grep -c '^send: ' "$TMP/j3")"
check "send failed 件数" 1 "$(grep -c 'send failed' "$TMP/j3")"
echo

# ---------------------------------------------------------------- 4
# 3行だけ流す。POLLHUP を POLLIN より先に見て break すると 0 行になる。
echo "4) 3行パイプ"
reset_procs
( sleep 6 | ./chat --host $PORT > "$TMP/h4" 2>&1 ) &
sleep 1
printf 'aaa\nbbb\nccc\n' | ./chat --join 127.0.0.1:$PORT > /dev/null 2>&1
code=$?
sleep 0.5
check "exit"   0 "$code"
check "受信行数" 3 "$(grep -c '^\[recv\]' "$TMP/h4")"
echo

# ---------------------------------------------------------------- 5
# macOS では /dev/null に POLLNVAL (0x20) が返る。POLLIN が立たないので
# 処理しないと poll が即座に返り続けて CPU 100% で回り続ける。
echo "5) 標準入力が /dev/null (POLLNVAL)"
reset_procs
./chat --host $PORT < /dev/null > "$TMP/h5" 2>&1 &
hostpid=$!
sleep 1
python3 -c "import socket,time; s=socket.create_connection(('127.0.0.1',$PORT)); time.sleep(3)" 2>/dev/null &
sleep 2
if kill -0 $hostpid 2>/dev/null; then
    check "終了したか" "終了" "生存(暴走の疑い)"
else
    wait $hostpid 2>/dev/null
    hostcode=$?                     # check を挟むと $? が上書きされるので先に取る
    check "終了したか" "終了" "終了"
    check "exit"       1      "$hostcode"
fi
echo

# ---------------------------------------------------------------- 6
# 片方を kill したら、もう片方が即座に検知して終わること。
echo "6) 片側を kill"
reset_procs
( sleep 15 | ./chat --host $PORT > "$TMP/h6" 2>&1 ) &
sleep 1
( sleep 15 | ./chat --join 127.0.0.1:$PORT > /dev/null 2>&1 ) &
sleep 1
pkill -f "chat --join 127.0.0.1:$PORT"
sleep 2
if grep -q 'peer closed' "$TMP/h6"; then
    check "相手の切断検知" "検知" "検知"
else
    check "相手の切断検知" "検知" "未検知"
fi
echo

# ----------------------------------------------------------------
echo "=================================="
printf ' PASS %d / FAIL %d\n' "$PASS" "$FAIL"
echo "=================================="
[ "$FAIL" -eq 0 ]
