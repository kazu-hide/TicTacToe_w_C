#ifndef BOARD_H
#define BOARD_H
#include "constants.h"

void initBoard(Board *board);
BOOL boardIsFull(Board *board);
BOOL isInBoard(int r, int c);
BOOL hasLegalMove(Board *board, char playerMark);
BOOL isWinMove(Board *board, int r, int c, char playerMark);
BOOL isMakingOverLine(Board *board, int r, int c, char playerMark);
BOOL isOpenLine(Board *board, Cell start, Cell end, Direction dir);
BOOL isAtLeastHalfOpenLine(Board *board, Cell start, Cell end, Direction dir);
LinePatterns countContinuousStonesWithGap(Board *board, int r, int c, int dx, int dy, char playerMark);
Cell getGapIdx(Board *board, LineInfo *line);
BOOL isFour(Board *board, LineInfo *line, char playerMark);
BOOL isMakingDoubleFour(Board *board, int row, int col, char playerMark);
BOOL isMakingGreatFour(Board *board, int r, int c, char playerMark);
BOOL isThree(Board *board, LineInfo *line, char playerMark);
BOOL isMakingDoubleThree(Board *board, int r, int c, char playerMark);
BOOL isProhibitedMove(Board *board, int r, int c, char playerMark);
#endif
