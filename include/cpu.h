#ifndef CPU_H
#define CPU_H

#include "constants.h"
#include "board.h"

Move getCpuMove(Game *game);
int negaMax(Board *board, int depth, char playerMark, int lastRow, int lastCol, int *bestRow, int *bestCol, int alpha, int beta);
EvaluationScores __evaluateStones(Board *board, char playerMark);
int evaluate(Board *board, char playerMark);

#endif
