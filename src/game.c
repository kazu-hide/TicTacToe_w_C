#include <stdio.h>
#include "ui.h"
#include "board.h"
#include "cpu.h"
#include "queue.h"
#include "game.h"

void playGame(int mode);
char getWinner(Board *board);
BOOL __isDrawGame(Board *board);

Game initGame(MODE mode) {
    Game game;
    initBoard(&(game.board));
    game.currentPlayer = PLAYER_X;
    game.gameState = GAME_PLAYING;
    game.winner = EMPTY_CELL;
    game.moveCount = 0;
    game.gameMode = mode;
    return game;
}

void switchPlayer(Game* game) {
    game->currentPlayer = (game->currentPlayer == PLAYER_X) ? PLAYER_O : PLAYER_X;
}

// row, col は「直前に打たれた手」。盤外の場合は打たれた手が無いとみなす。
BOOL isGameOver(Board *board, int row, int col, char player) {
    if (isInBoard(row, col) && isWinMove(board, row, col, player))
        return TRUE;
    return boardIsFull(board);
}

BOOL isValidMove(Board* board, int row, int col, char playerMark) {
    if (row < 1 || BOARD_ROWS < row || col < 1 || BOARD_COLUMNS < col) {
        printf("Error: Row and Column must be between 1 and %d.\n", BOARD_ROWS);
        return FALSE;
    }

    if (board->cells[row][col] != EMPTY_CELL) {
        printf("Error: The %d, %d is already marked.\n", row, col);
        return FALSE;
    }

    if (isProhibitedMove(board, row, col, playerMark)) {
        printf("Error: The %d, %d is prohibited move.\n", row, col);
        return FALSE;
    }
    return TRUE;
}

Move getPlayerMove(Game *game) {
    while (TRUE)
    {
        Move move = getPlayerInput();
        printf("row %d, col %d", move.row, move.col);
        if (isValidMove(&game->board, move.row, move.col, game->currentPlayer))
        {
            return move;
        }
    }
}

void applyMove(Game* game, Move move) {
    game->board.cells[move.row][move.col] = game->currentPlayer;
    Hand hand = {.move.row = move.row, .move.col = move.col, .player = game->currentPlayer};
    game->handHistory[game->moveCount] = hand;
    game->moveCount++;
}

void updateGameState(Game *game, Move move){
    if (isWinMove(&game->board, move.row, move.col, game->currentPlayer)) {
        game->gameState = GAME_WIN;
        game->winner = game->currentPlayer;
    }
    else if (boardIsFull(&game->board)) {
        game->gameState = GAME_DRAW;
    }
}

// 1ターンの処理
void playTurn(Game* game) {
    Move move;
    printGameStatus(game->moveCount + 1, game->currentPlayer);

    if (
        (
            game->gameMode == PLAYER_CPU &&
            game->currentPlayer == PLAYER_O) ||
        game->gameMode == CPU_CPU) {
        move = getCpuMove(game);
    } else {
        move = getPlayerMove(game);
    }

    applyMove(game, move);
    updateGameState(game, move);
    printBoard(game);
    announceResult(game);
}

void playGame(int mode)
{

    Game game = initGame(mode);

    printf("\n\tRenju Game Started!\n\n");
    printBoard(&game);

    while (game.gameState == GAME_PLAYING) {
        playTurn(&game);

        if (game.gameState == GAME_PLAYING) {
            switchPlayer(&game);
        }
    }
}

void runGameLoop() {
    while (TRUE) {
        MODE mode = selectGameMode();

        playGame(mode);

        if (!askForRematch()) {
            displayThanksMessage();
            break;
        }
    }
}
