#ifndef UI_H
#define UI_H
#include "constants.h"
#include "board.h"


Move getPlayerInput();
BOOL isQuitMove(Move move);
void displayThanksMessage(void);
void announceNoLegalMove(char player);
void printBoard(Game *game);
void announceResult(const Game *game);
void printGameStatus(int turnCounts, char player);
BOOL askForRematch(void);
MODE selectGameMode();
#endif
