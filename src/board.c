#include <stdio.h>
#include "utils.h"
#include "board.h"

static const Direction DIRS[4] = {
    {1, 0},   // 垂直方向 (↓)
    {0, 1},   // 水平方向 (→)
    {1, 1},   // 右下がり斜め (↘)
    {1, -1}   // 右上がり斜め (↗)
};

static const int dirLength = 4;

BOOL isProhibitedMove(Board *board, int r, int c, char playerMark);

void initBoard(Board *board) {
    size_t i, j;
    for (i = 1; i <= BOARD_ROWS; i++) {
        for (j = 1; j <= BOARD_COLUMNS; j++) {
            board->cells[i][j] = EMPTY_CELL;
        }
    }
}

BOOL boardIsFull(Board *board) {
    size_t i, j;
    for (i = 1; i <= BOARD_ROWS; i++) {
        for (j = 1; j <= BOARD_COLUMNS; j++) {
            if (board->cells[i][j] == EMPTY_CELL)
                return FALSE;
        }
    }
    return TRUE;
}

BOOL isInBoard(int r, int c) {
    if (1 <= r && r <= BOARD_ROWS && 1 <= c && c <= BOARD_COLUMNS)
        return TRUE;
    return FALSE;
}

// 特定の方向の連続した石の数を数える共通関数
static int countContinuousStones(Board *board, int r, int c, int dx, int dy, char playerMark) {
    int length = 1;  // 置いた石から開始
    
    // 両方向（正負）に探索
    for (int reverse = 0; reverse <= 1; reverse++) {
        int nextX = r + (reverse == 0 ? -dx : dx);
        int nextY = c + (reverse == 0 ? -dy : dy);
        
        while (isInBoard(nextX, nextY) && board->cells[nextX][nextY] == playerMark) {
            length++;
            nextX += (reverse == 0 ? -dx : dx);
            nextY += (reverse == 0 ? -dy : dy);
        }
    }
    
    return length;
}

// r, cに石を置いた場合の特定の方向の連続した石を1つのGAPありで数える
LinePatterns countContinuousStonesWithGap(Board *board, int r, int c, int dx, int dy, char playerMark) {
    LinePatterns result = {.pattern = 0};
    for (int gapSide = 0; gapSide <= 1; gapSide++) {  // 0:左側, 1:右側
        int minIdx = 0, maxIdx = 0;
        int length = 1;  // 置いた石から開始
        BOOL gapUsed = FALSE;

        for (int reverse = 0; reverse <= 1; reverse++) {
            int nextX = r + (reverse == 0 ? -dx : dx);
            int nextY = c + (reverse == 0 ? -dy : dy);
            int relativePos = (reverse == 0 ? -1 : 1);
            
            while (isInBoard(nextX, nextY)) {
                if (board->cells[nextX][nextY] == playerMark)
                {
                    length++;
                    minIdx = min(minIdx, relativePos);
                    maxIdx = max(maxIdx, relativePos);
                // 意図した方向のgapの場合は、有効なgapかどうかをチェック。
                }
                else if (!gapUsed &&
                         board->cells[nextX][nextY] == EMPTY_CELL &&
                         ((gapSide == 0 && reverse == 0) || (gapSide == 1 && reverse == 1)))
                {
                    // gapをまだ1度も通っておらず、gapの次の石が自分の石の場合、数え続ける。
                    //　その他の場合は、数えるのをやめる。
                    nextX += (reverse == 0 ? -dx : dx);
                    nextY += (reverse == 0 ? -dy : dy);
                    relativePos += (reverse == 0 ? -1 : 1);
                    if (isInBoard(nextX, nextY) && board->cells[nextX][nextY] == playerMark) {
                        gapUsed = TRUE;
                        continue;
                    } else
                        break;
                }
                else
                {
                    break;
                }
                nextX += (reverse == 0 ? -dx : dx);
                nextY += (reverse == 0 ? -dy : dy);
                relativePos += (reverse == 0 ? -1 : 1);
            }
        }
        // gapを使っている場合、新しい長さとして保存
        // gapを使っていない場合、両サイドともgapがない場合に限り1度だけ保存
        // 片側のみgapがあるのは、長さパターン1つとなる
        if (gapUsed || (result.pattern == 0 && gapSide == 1)) {
            result.lines[result.pattern].start.r = r + dx * minIdx;
            result.lines[result.pattern].start.c = c + dy * minIdx;
            result.lines[result.pattern].end.r = r + dx * maxIdx;
            result.lines[result.pattern].end.c = c + dy * maxIdx;
            result.lines[result.pattern].dir.dx = dx;
            result.lines[result.pattern].dir.dy = dy;
            result.lines[result.pattern].hasGap = gapUsed;
            result.lines[result.pattern].length = length;
            result.pattern++;
        }
    }
    return result;
}

BOOL isWinMove(Board *board, int r, int c, char playerMark) {
    if (playerMark == EMPTY_CELL)
        return FALSE;
        
    for (int dir = 0; dir < dirLength; dir++) {
        int length = countContinuousStones(board, r, c, DIRS[dir].dx, DIRS[dir].dy, playerMark);
        
        if ((playerMark == PLAYER_O && length >= 5) || length == 5)
            return TRUE;
    }
    return FALSE;
}

// 禁じ手である、長連の判定関数
BOOL isMakingOverLine(Board *board, int r, int c, char playerMark) {
    if (playerMark != PLAYER_X)
        return FALSE;
        
    for (int dir = 0; dir < dirLength; dir++) {
        int length = countContinuousStones(board, r, c, DIRS[dir].dx, DIRS[dir].dy, playerMark);
        
        if (length >= 6)
            return TRUE;
    }
    return FALSE;
}

// ちょうど5連の並びを作っているかの判定関数
BOOL isMakingExactFive(Board *board, int r, int c, char playerMark) {
    for (int dir = 0; dir < dirLength; dir++)
    {
        int length = countContinuousStones(board, r, c, DIRS[dir].dx, DIRS[dir].dy, playerMark);
        if (length == 5)
            return TRUE;
    }
    return FALSE;
}

// 両側が空いているラインかどうかを確認
BOOL isOpenLine(Board *board, Cell start, Cell end, Direction dir) {

    // 両サイドが盤面外の場合
    if (!isInBoard(start.r - dir.dx, start.c - dir.dy) || !isInBoard(end.r + dir.dx, end.c + dir.dy))
        return FALSE;
    return (
        board->cells[start.r - dir.dx][start.c - dir.dy] == EMPTY_CELL && 
        board->cells[end.r + dir.dx][end.c + dir.dy] == EMPTY_CELL
    );
}

BOOL isAtLeastHalfOpenLine(Board *board, Cell start, Cell end, Direction dir) {
    // ラインとして少なくとも片側が空いているかどうかの判定関数
    // 盤外のセルは「空いている」とはみなさない。
    // 各サイドごとに盤内かを確認してから参照する（片側だけ盤外のケースがあるため）

    Cell prev = {.r = start.r - dir.dx, .c = start.c - dir.dy};
    Cell next = {.r = end.r + dir.dx, .c = end.c + dir.dy};

    BOOL isPrevOpen = isInBoard(prev.r, prev.c) &&
                      board->cells[prev.r][prev.c] == EMPTY_CELL;
    BOOL isNextOpen = isInBoard(next.r, next.c) &&
                      board->cells[next.r][next.c] == EMPTY_CELL;

    return (isPrevOpen || isNextOpen) ? TRUE : FALSE;
}


Cell getGapIdx(Board *board, LineInfo *line) {
    Cell result = {.r = -1, .c = -1};
    
    // gapがない場合は早期リターン
    if (!line->hasGap)
        return result;
        
    // 開始位置から終了位置まで探索
    // ただし両端は探索しない
    int x = line->start.r + line->dir.dx;
    int y = line->start.c + line->dir.dy;
    int endX = line->end.r;
    int endY = line->end.c;
    
    while (x != endX || y != endY) {
        if (board->cells[x][y] == EMPTY_CELL) {
            return (Cell){.r = x, .c = y};
        }
        x += line->dir.dx;
        y += line->dir.dy;
    }
    
    return result;
}

BOOL isFour(Board *board, LineInfo *line, char playerMark) {
    /* 
    四の定義は、一個の石を加えることで五にできるもの
    そのため、両サイドが塞がれているものは四ではない。
    また一個石を置くと5を超えてしまうものは、五にできていないため、四ではない
    */

    if (line->length != 4)
        return FALSE;

    if (!isAtLeastHalfOpenLine(board, line->start, line->end, line->dir))
        return FALSE;
    
    // gapがある場合は、そこに石を置いて五になるかを判定
    if (line->hasGap) {
        Cell gap = getGapIdx(board, line);
        return isMakingExactFive(board, gap.r, gap.c, playerMark);
    }

    // gapがない場合は、両側の空いているマスに置いて五になるかを確認
    Cell leftEdge = {
        .r = line->start.r - line->dir.dx, 
        .c = line->start.c - line->dir.dy
    };
    Cell rightEdge = {
        .r = line->end.r + line->dir.dx, 
        .c = line->end.c + line->dir.dy
    };

    // 両端のマスが盤内かつ空いているかをチェック
    BOOL canPlaceLeft = isInBoard(leftEdge.r, leftEdge.c) && 
                       board->cells[leftEdge.r][leftEdge.c] == EMPTY_CELL;
    BOOL canPlaceRight = isInBoard(rightEdge.r, rightEdge.c) && 
                        board->cells[rightEdge.r][rightEdge.c] == EMPTY_CELL;

    // どちらかの端に置いて五になるかをチェック
    return (canPlaceLeft && isMakingExactFive(board, leftEdge.r, leftEdge.c, playerMark)) ||
           (canPlaceRight && isMakingExactFive(board, rightEdge.r, rightEdge.c, playerMark));
}

// r, cに石を置いたときに、四四を作るかどうかの判定関数
BOOL isMakingDoubleFour(Board *board, int r, int c, char playerMark){
    //一時的にボードを作り石を置いてみる
    Board tmpBoard = *board;
    tmpBoard.cells[r][c] = playerMark;
    
    int fourLineCount = 0;

    // 4方向それぞれについて四を数える
    for (int dir = 0; dir < dirLength; dir++) {
        LinePatterns linePatterns = countContinuousStonesWithGap(
            &tmpBoard, r, c, DIRS[dir].dx, DIRS[dir].dy, playerMark
        );
        
        for (int i = 0; i < linePatterns.pattern; i++) {
            if (isFour(&tmpBoard, &linePatterns.lines[i], playerMark)) {
                fourLineCount++;
                if (fourLineCount > 1)
                    return TRUE;
            }
        }
    }
    
    return FALSE;
}

// 達四であるかどうかの判定関数
// 達四とは、五にする場所が2つある四のこと
BOOL isMakingGreatFour(Board *board, int r, int c, char playerMark) {
    BOOL result = FALSE;

    Board tmpBoard = *board;
    tmpBoard.cells[r][c] = playerMark;

    for (int dir = 0; dir < dirLength; dir++)
    {
        LinePatterns linePatterns = countContinuousStonesWithGap(
            &tmpBoard, r, c, DIRS[dir].dx, DIRS[dir].dy, playerMark
        );
        
        for (int i = 0; i < linePatterns.pattern; i++) {
            LineInfo line = linePatterns.lines[i];
            // 黒番の場合長さが4ではない、またギャップがあると達四になり得ないため、early return
            if (playerMark == PLAYER_X && (line.length != 4 || line.hasGap == TRUE))
                continue;

            // LINEの両サイドのセル
            int prevX = line.start.r - line.dir.dx;
            int prevY = line.start.c - line.dir.dy;

            int nextX = line.end.r + line.dir.dx;
            int nextY = line.end.c + line.dir.dy;

            // gapがあって、条件を満たす特殊ケース
            // 白手番かつgapを埋めて五になり、さらに両側のどちらかに石を置いて五になるケース
            if (playerMark == PLAYER_O && line.length >= 5 && line.hasGap == TRUE) {
                Cell gap = getGapIdx(&tmpBoard, &line);

                if (
                    isWinMove(&tmpBoard, gap.r, gap.c, playerMark) &&
                    (
                        (
                            isInBoard(prevX, prevY) &&
                            isWinMove(&tmpBoard, prevX, prevY, playerMark)) ||
                        (
                            isInBoard(nextX, nextY) &&
                            isWinMove(&tmpBoard, nextX, nextY, playerMark))
                    )
                )
                    result = TRUE;
            }
            // 以下は、白、黒共有ケース
            // 両側のマスが空いている
            if (!isOpenLine(&tmpBoard, line.start, line.end, line.dir))
                continue;

            // 白石の場合、5以上であれば五、だが黒石は5を超えると長連となり五とみなされない
            if (
                isWinMove(&tmpBoard, prevX, prevY, playerMark) == TRUE &&
                isWinMove(&tmpBoard, nextX, nextY, playerMark) == TRUE)
                result = TRUE;
        }
    }
    return result;
}


BOOL isThree(Board *board, LineInfo *line, char playerMark) {
    /* 
    三であるかどうかの判定関数
    三とは、達四にする手段のある3のこと
    */
    if (line->length != 3)
        return FALSE;

    if (!isAtLeastHalfOpenLine(board, line->start, line->end, line->dir))
        return FALSE;

    // gapがある場合は、そこに石を置いて達四になるかを判定
    if (line->hasGap) {
        Cell gap = getGapIdx(board, line);
        return (
            !isProhibitedMove(board, gap.r, gap.c, playerMark) && 
            isMakingGreatFour(board, gap.r, gap.c, playerMark)
        );
    }

    // gapがない場合は、両側の空いているマスに置いて五になるかを確認
    Cell leftEdge = {
        .r = line->start.r - line->dir.dx, 
        .c = line->start.c - line->dir.dy
    };
    Cell rightEdge = {
        .r = line->end.r + line->dir.dx, 
        .c = line->end.c + line->dir.dy
    };

    // 両端のマスが盤内かつ空いているかをチェック
    BOOL canPlaceLeft = isInBoard(leftEdge.r, leftEdge.c) && 
                       board->cells[leftEdge.r][leftEdge.c] == EMPTY_CELL;
    BOOL canPlaceRight = isInBoard(rightEdge.r, rightEdge.c) && 
                        board->cells[rightEdge.r][rightEdge.c] == EMPTY_CELL;

    // どちらかの端に置いて達四になるかをチェック
    return (
            (
                canPlaceLeft && 
                !isProhibitedMove(board, leftEdge.r, leftEdge.c, playerMark) &&
                isMakingGreatFour(board, leftEdge.r, leftEdge.c, playerMark)
            ) ||
           (
                canPlaceRight && 
                !isProhibitedMove(board, rightEdge.r, rightEdge.c, playerMark) &&
                isMakingGreatFour(board, rightEdge.r, rightEdge.c, playerMark)
            )
        );
}

BOOL isMakingDoubleThree(Board *board, int r, int c, char playerMark){
    // r, cに石を置いたときに、三三を作るかどうかの判定関数

    //一時的にボードを作り石を置いてみる
    Board tmpBoard = *board;
    tmpBoard.cells[r][c] = playerMark;
    
    int threeLineCount = 0;

    // 4方向それぞれについて四を数える
    for (int dir = 0; dir < dirLength; dir++) {
        LinePatterns linePatterns = countContinuousStonesWithGap(
            &tmpBoard, r, c, DIRS[dir].dx, DIRS[dir].dy, playerMark
        );
        
        for (int i = 0; i < linePatterns.pattern; i++) {
            if (isThree(&tmpBoard, &linePatterns.lines[i], playerMark)) {
                threeLineCount++;
                if (threeLineCount > 1)
                    return TRUE;
            }
        }
    }
    
    return FALSE;
}

// 禁じ手かどうかの判定関数
BOOL isProhibitedMove(Board *board, int r, int c, char playerMark) {
    if (playerMark != PLAYER_X)
        return FALSE;
    
    // 五がある場合、五が優先される
    if (isWinMove(board, r, c, playerMark))
        return FALSE;

    return (
        isMakingOverLine(board, r, c, playerMark) ||
        isMakingDoubleFour(board, r, c, playerMark) ||
        isMakingDoubleThree(board, r, c, playerMark)
        );
}
