#ifndef CONSTANTS_H
#define CONSTANTS_H

#define WIN_LENGTH 5

#define BOARD_ROWS 9
#define BOARD_COLUMNS 9

#define PLAYER_X 'X'
#define PLAYER_O 'O'
#define EMPTY_CELL ' '

#define QUEUE_SIZE 100

#define NEGA_MAX_DEPTH 3

// negaMax の alpha / beta の初期値。評価値が取りうる範囲より十分大きい値
#define SCORE_INFINITY 9999999

// 決着した局面のスコア。ヒューリスティックな評価値の合計より必ず大きい
#define TERMINAL_WIN_SCORE 1000000

// ライン長さスコア
#define WIN_POINTS 99999
#define OPEN_FOUR_POINTS 5000
#define CLOSED_FOUR_POINTS 500
#define OPEN_THREE_POINTS 500
#define CLOSED_THREE_POINTS 100
#define OPEN_TWO_POINTS 100
#define CLOSED_TWO_POINTS 25

// ポジショニングスコア
#define CENTER_START_ROW (BOARD_ROWS / 3)
#define CENTER_END_ROW ((BOARD_ROWS * 2) / 3)
#define CENTER_START_COL (BOARD_COLUMNS / 3)
#define CENTER_END_COL ((BOARD_COLUMNS * 2) / 3)
#define CENTER_MULTIPLIER 2

typedef enum 
{
    TRUE = 1,
    FALSE = 0
} BOOL;

typedef enum {
    GAME_PLAYING,
    GAME_WIN,     
    GAME_DRAW,   
    GAME_QUIT
} GameState;

typedef struct {
    char cells[BOARD_ROWS + 1][BOARD_COLUMNS + 1];
    int lastRow, lastCol;
} Board;

typedef struct {
    int r;
    int c;
} Cell;

typedef enum {
    NORMAL,
    LAST_MOVE,
    PROHIBITED
} CellDisplayType;

typedef struct {
    int row;
    int col;
} Move;

typedef struct {
    Move move;
    char player; // PLAYER_X or PLAYER_O
} Hand;

typedef enum
{
    PLAYER_PLAYER = 1,
    PLAYER_CPU = 2,
    CPU_CPU = 3
} MODE;

typedef struct {
    Board board;
    char currentPlayer;
    GameState gameState;
    char winner;
    int moveCount;
    Hand handHistory[BOARD_ROWS * BOARD_COLUMNS];
    MODE gameMode;
} Game;

// 方向を表す構造体
typedef struct
{
    int dx;  // x方向の変化量
    int dy;  // y方向の変化量
} Direction;

typedef struct {
    Cell start;
    Cell end;
    int length;
    Direction dir;
    BOOL hasGap;
} LineInfo;

// gap込みで長さを測ると、gapの位置によって、同じラインが最大２パターンの長さを持つ。
typedef struct {
    LineInfo lines[2];  // 最大2パターンを格納
    int pattern; // 見つかったパターンの数（1または2）
} LinePatterns;

typedef struct {
    int lengthScore;      // 連続した石の評価
    int positionScore;    // 位置の価値評価
    int patternScore;     // その他パターンの評価
} EvaluationScores;

#endif
