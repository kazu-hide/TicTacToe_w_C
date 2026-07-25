#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "ui.h"
#include "board.h"


static void clearInputBuffer(void) {
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
}

static void printColumnNumbers(void) {
    printf("\t");
    for (int k = 1; k <= BOARD_COLUMNS; k++) {
        printf("%d   ", k);
    }
    printf("\n\n");
}

// セパレータ文字列の作成
static void createSeparator(char *separator) {
    separator[0] = '\0';
    strcat(separator, "\t");
    for (int j = 1; j < BOARD_COLUMNS; j++) {
        strcat(separator, "- + ");
    }
    strcat(separator, "-");
}

// セルの表示（最後の手は緑色で表示, 禁じ手は赤の*で表示）
static void printCell(char cell, CellDisplayType displayType) {
    switch(displayType) {
        case LAST_MOVE:
            printf("\x1b[32m%c\x1b[39m", cell);
            break;
        case PROHIBITED:
            if (cell == EMPTY_CELL) {
                    printf("\x1b[31m*\x1b[39m");
                } else {
                    printf("%c", cell);
                }
                break;
        default:
            printf("%c", cell);
            break;
        }
}

// セルの表示種別を決定する
static CellDisplayType getCellDisplayType(Board *board, const Hand *lastHand,
                                          int row, int col, char nextPlayer) {
    if (row == lastHand->move.row && col == lastHand->move.col)
        return LAST_MOVE;

    if (board->cells[row][col] == EMPTY_CELL &&
        isProhibitedMove(board, row, col, nextPlayer))
        return PROHIBITED;

    return NORMAL;
}

void printBoard(Game *game) {
    Board *board = &game->board;
    Hand lastHand = game->handHistory[game->moveCount - 1];

    char nextPlayer = (game->currentPlayer == PLAYER_X) ? PLAYER_O : PLAYER_X;

    printColumnNumbers();
    
    char separator[4 * BOARD_COLUMNS + 4];
    createSeparator(separator);

    for (int i = 1; i <= BOARD_ROWS; i++) {
        printf("%d\t", i);

        for (int j = 1; j <= BOARD_COLUMNS; j++) {
            printCell(board->cells[i][j], getCellDisplayType(board, &lastHand, i, j, nextPlayer));

            // 最後の列の後には区切り文字を入れない
            if (j < BOARD_COLUMNS) {
                printf(" | ");
            }
        }
        printf("\n");

        // セパレータの表示
        if (i < BOARD_ROWS) {
            printf("%s\n", separator);
        }
    }
    printf("\n\n");
}



void printGameStatus(int turnCounts, char player) {
    printf("Turn %d, %c's turn.\n",turnCounts, player);    
}

void displayWelcomeMessage(void) {
    printf("Welcome to Renju (連珠)!\n");
    printf("================================\n\n");
}

void displayGameRules(void) {
    printf("Game Rules:\n");
    printf("1. Black (X) plays first\n");
    printf("2. Place pieces to get 5 in a row\n");
    printf("3. Black has forbidden moves (三三, 四四, 長連)\n");
    printf("4. Enter moves as 'row,col' (e.g., '5,5')\n\n");
}


void displayThanksMessage(void) {
    printf("\n\tThanks for playing!\n\n");
    printf("================================\n");
}

Move getPlayerInput() {
    Move move = {0, 0};
    char input[8];

    while (TRUE)
    {
        printf("Please input row,col (or 'q' to quit): ");
        
        if (fgets(input, sizeof(input), stdin) == NULL) {
            continue;
        }

        if (input[0] == 'q' || strcmp(input, "quit\n") == 0) {
            printf("\tGame Ended.\n");
            exit(0);
        }

        if (sscanf(input, " %d , %d ", &move.row, &move.col) != 2 && sscanf(input, " %d %d ", &move.row, &move.col) != 2) {
            continue;
        }
        
        return move;
    }
}

MODE selectGameMode() {
    int mode;
    displayWelcomeMessage();
    displayGameRules();

    while (1) {
        printf("Select game mode:\n");
        printf("1. Player vs Player\n");
        printf("2. Player vs CPU\n");
        printf("3. CPU vs CPU\n");
        printf("Enter mode (1-3): ");
        
        if (scanf("%d", &mode) == 1 && mode >= PLAYER_PLAYER && mode <= CPU_CPU) {
            clearInputBuffer();
            return (MODE)mode;
        }
        
        printf("Invalid input. Please try again.\n");
        clearInputBuffer();
    }
}

void announceResult(const Game* game) {
    
    switch (game->gameState) {
        case GAME_PLAYING:
            printf("Player %c placed at: %d, %d\n", game->currentPlayer, game->handHistory[game->moveCount - 1].move.row, game->handHistory[game->moveCount - 1].move.col);
            break;
        case GAME_WIN:
            printf("Congratulations! Player %c wins!\n", game->winner);
            break;
        case GAME_DRAW:
            printf("The game ended in a draw!\n");
            break;
        default:
            printf("[Error] Unexpected game state!\n");
    }
}

BOOL askForRematch(void) {
    char response;
    printf("\nWould you like to play again? (y/n): ");
    
    while (1) {
        scanf(" %c", &response);
        clearInputBuffer();
        
        if (response == 'y' || response == 'Y') {
            return TRUE;
        } else if (response == 'n' || response == 'N') {
            return FALSE;
        }
        
        printf("Invalid input. Please enter 'y' or 'n': ");
    }
}
