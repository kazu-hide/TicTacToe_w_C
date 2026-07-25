# 連珠のルールと本実装の仕様

このドキュメントは、実装上の判断が必要になったルールについて、公式ルールを調べた結果と、
本実装が採用する仕様を確定させたものです。

論点: **黒に着手できる空点が1つも無くなった場合 (残る空点が全て禁じ手) にどうなるか。**

---

## 1. 前提となるルール

### 1.1 禁じ手 (黒のみ)

黒には三三・四四・長連の3種類の禁じ手がある。白には禁じ手が一切無い。

> 三三禁・四四禁・長連の三種類で、先手の黒のみ課せられます。白には一切の禁じ手はありません
> — [連珠とは (東海連珠会)](http://tokai-renjukai.pya.jp/info/Renju.html)

### 1.2 五を同時に作る場合は禁じ手にならない

RIF (Renju International Federation) 国際ルールでは、禁じ手による白の勝ちは
「**同時に五を作っていない場合に限る**」と条件づけられている。

> The game is won for white if black **without at the same time attaining five in a row**:
> a) makes an overline b) makes a double-four c) makes a double-three
> — [International Rules of Renju, RIF](https://www.renju.net/rifrules/)

本実装の `isProhibitedMove()` は五を作る手を先に判定して `FALSE` を返しており、
この規定と一致している。

### 1.3 引き分けの条件

> 10.1 when all the intersection of the board are occupied;
> 10.2 by agreement between the both players;
> 10.3 when both the players (after each other) pass;
> 10.4 when both the players time has ended.
> — [International Rules of Renju, RIF](https://www.renju.net/rifrules/)

本実装に関係するのは 10.1 のみ (合意・パス・持ち時間は実装していない)。

---

## 2. 論点への結論

### 2.1 黒に着手できる空点が無い場合 → **黒の負け (白の勝ち)**

**根拠1: 公式ルールに「黒が打てない」という状態は存在しない。**

RIF ルールは禁じ手を「打てない手」ではなく「**打つと負ける手**」として定義している。
空点はすべて着手可能であり、禁点に打った時点で白の勝ちになる。
したがって残る空点が全て禁じ手であれば、黒はそのいずれかに打つしかなく、必ず負ける。

**根拠2: 日本の連珠では「打たされても負け」と明記されている。**

> 黒は五連を並べる前の「三三」と「四四」と六目以上並んだ「長連」が『禁手』で
> 「**打っても、打たされても負け**」になります
> — [連珠とは (東海連珠会)](http://tokai-renjukai.pya.jp/info/Renju.html)

**根拠3: 禁点に追い込むことは白の正当な戦術として位置づけられている。**

> The aspect of "forbidden moves" brings a totally new tactical tool into renju game -
> **trapping the forbidden points** - which is a weapon only for the white player.
> — [What is Renju? (RenjuNet)](https://www.renju.net/rules/)

つまりこの局面は例外的な異常系ではなく、**白が狙って作りにいく勝ち筋**である。
「打つ場所が無いので引き分け」とするのは、白の勝ちを取り上げてしまうため誤り。

### 2.2 白に着手できる空点が無い場合 → **引き分け**

白には禁じ手が無いため、「白が打てない」= 盤面に空点が1つも無い、と同値。
これは RIF 10.1 の満局にあたるので引き分け。

### 2.3 両者とも打てない場合 → **引き分け**

2.2 と同じく満局。既存の `boardIsFull()` による引き分け判定がそのまま該当する。

---

## 3. 本実装が採用する仕様 (確定)

現在の実装は禁じ手を `isValidMove()` で弾き、「打てない手」として扱っている。
これは形式上 RIF と異なるが、**黒が禁点に打てば必ず負ける以上、最善を尽くす限り
両者は戦略的に等価**であり、この UI を変更する必要は無い
(README に記載済みの「禁じ手は打つ事ができない」という挙動も維持できる)。

ただし「黒に合法手が無い」局面だけは結果が未定義なので、以下を仕様として確定する。

> **手番側に合法手 (空点かつ禁じ手でない点) が1つも存在しない場合、その手番側の負けとする。**
>
> - 黒の場合: 禁点に打たされたものとみなし、白の勝ち。
> - 白の場合: 白に禁じ手は無いため、これは満局と同値。引き分け判定が先に成立するため
>   このケースには到達しない。

### 3.1 判定順序

1. 直前の手で五ができている → その手番の勝ち (`GAME_WIN`)
2. 盤面が満局 → 引き分け (`GAME_DRAW`)
3. 手番側に合法手が無い → 手番側の負け (`GAME_WIN` / `winner` は相手)
4. 上記以外 → 続行 (`GAME_PLAYING`)

2 を 3 より先に評価することで、白に合法手が無いケースが自動的に引き分けになる。

### 3.2 実装への影響

現状、この局面に到達すると次の不具合が起きる。

- 人間が黒の場合: `getPlayerMove()` が入力を却下し続けて進行不能になる
- CPU が黒の場合: `getCpuMove()` が `(0, 0)` を返し、盤面に反映されないまま
  ターンだけが進み、ゲームが終わらない

実装は別途 issue で対応する。

---

## 4. 調査したが本ドキュメントの対象外とした点

- **RIF 9.3 の「許される三三」**: 少なくとも一方の三が (長連や四四を作らずに) 達四に
  できない場合、その三三は禁じ手にならない。本実装の `isThree()` は達四にできるかを
  判定しているため、この規定は既に考慮されている。
- **開局規定 (RIF opening rule, 五珠交替など)**: 本実装は自由布石であり対象外。
- **合意・パス・持ち時間による引き分け (RIF 10.2〜10.4)**: 本実装には該当機能が無い。

---

## 参考文献

- [International Rules of Renju — RenjuNet (RIF)](https://www.renju.net/rifrules/)
- [What is Renju? — RenjuNet](https://www.renju.net/rules/)
- [連珠とは — 東海連珠会](http://tokai-renjukai.pya.jp/info/Renju.html)
- [連珠 — Wikipedia (日本語)](https://ja.wikipedia.org/wiki/連珠)
