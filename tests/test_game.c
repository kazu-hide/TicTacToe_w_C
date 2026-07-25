#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "../include/board.h"
#include "../include/game.h"
#include "test_utils.h"
#include "test_game.h"

void testGameInitialization(TestResults* results) {
    test_begin("GameInitialization");
    
    Game game = initGame(PLAYER_PLAYER);
    
    test_assert(game.currentPlayer == PLAYER_X,
                "Game should start with PLAYER_X", results);
    test_assert(game.gameState == GAME_PLAYING,
                "Game should start in PLAYING state", results);
    test_assert(game.moveCount == 0,
                "Hand count should start at 0", results);
    test_assert(game.gameMode == PLAYER_PLAYER,
                "Game mode should be the PLAYER_PLAYER", results);
                
    test_end("GameInitialization");
}

void testPlayerSwitch(TestResults* results) {
    test_begin("PlayerSwitch");
    
    Game game = initGame(PLAYER_PLAYER);
    
    switchPlayer(&game);
    test_assert(game.currentPlayer == PLAYER_O,
                "Should switch to PLAYER_O", results);
    
    switchPlayer(&game);
    test_assert(game.currentPlayer == PLAYER_X,
                "Should switch back to PLAYER_X", results);
    
    test_end("PlayerSwitch");
}

void testPlaceMoveExpected(TestResults* results) {
    test_begin("PlaceMoveExpected");

    Board board = __prepareBoard();
    test_assert(isValidMove(&board, 5, 5, PLAYER_X) == TRUE,
                "Should allow move to empty center position", results);

    test_assert(isValidMove(&board, 1, 1, PLAYER_X) == TRUE,
                "Should allow move to top-left corner", results);
    
    test_assert(isValidMove(&board, BOARD_ROWS, 1, PLAYER_X) == TRUE,
                "Should allow move to bottom-left corner", results);

    test_assert(isValidMove(&board, 1, BOARD_COLUMNS, PLAYER_X) == TRUE,
                "Should allow move to top-right corner", results);

    test_assert(isValidMove(&board, BOARD_ROWS, BOARD_COLUMNS, PLAYER_X) == TRUE,
                "Should allow move to bottom-right corner", results);

    test_end("PlaceMoveExpected");
}

void testPlaceMoveFailedAlreadyMarked(TestResults* results) {
    Board board = __prepareBoard();
    board.cells[5][5] = PLAYER_X;

    test_assert(isValidMove(&board, 5, 5, PLAYER_X) == FALSE,
                "Should not allow move to already marked position", results);
}

void testPlaceMoveFailedOutOfRange(TestResults* results) {
    Board board = __prepareBoard();
    test_assert(isValidMove(&board, BOARD_ROWS + 1, BOARD_COLUMNS, PLAYER_X) == FALSE,
                "Should not allow move out of range", results);
}

void testValidateInputExpectedRange(TestResults* results) {
    test_begin("ValidateInputExpectedRange");

    printf("000000");

    Board board = __prepareBoard();

    int row = 5;
    int col = 5;
    test_assert(isValidMove(&board, row, col, PLAYER_X) == TRUE,
                "Should allow move in valid range", results);

    int rowEdgeBegin = 1;
    int colEdgeBegin = 1;
    test_assert(isValidMove(&board, rowEdgeBegin, colEdgeBegin, PLAYER_X) == TRUE,
                "Should allow move to top-left corner", results);

    int rowEdgeEnd = BOARD_ROWS;
    int colEdgeEnd = BOARD_COLUMNS;
    test_assert(isValidMove(&board, rowEdgeEnd, colEdgeEnd, PLAYER_X) == TRUE,
                "Should allow move to bottom-right corner", results);

    test_end("ValidateInputExpectedRange");
}

void testValidateInputFailedOutOfRange(TestResults* results) {
    test_begin("ValidateInputFailedOutOfRange");

    printf("000000");

    Board board = __prepareBoard();

    int zeroRow = 0;
    int zeroCol = 0;

    test_assert(isValidMove(&board, zeroRow, zeroCol, PLAYER_X) == FALSE,
                "Should not allow move in zero range", results);
    
    int negativeInt = -1;
    int randInt = 4;
    test_assert(isValidMove(&board, negativeInt, randInt, PLAYER_X) == FALSE,
                "Should not allow move in negative range", results);

    int ToobigInt = 123490;
    test_assert(isValidMove(&board, ToobigInt, randInt, PLAYER_X) == FALSE,
                "Should not allow move in too big range", results);

    test_end("ValidateInputFailedOutOfRange");
}

void testValidateInputFailedNotEmpty(TestResults* results) {
    test_begin("ValidateInputFailedNotEmpty");

    Board board = __prepareBoard();

    int row = 5;
    int col = 5;

    board.cells[row][col] = PLAYER_O;
    test_assert(isValidMove(&board, row, col, PLAYER_X) == FALSE,
                "Should not allow move to already marked position", results);

    int rowEdgeBegin = 1;
    int colEdgeBegin = 1;
    board.cells[rowEdgeBegin][colEdgeBegin] = PLAYER_O;
    test_assert(isValidMove(&board, rowEdgeBegin, colEdgeBegin, PLAYER_X) == FALSE,
                "Should not allow move to already marked position", results);

    int rowEdgeEnd = BOARD_ROWS;
    int colEdgeEnd = BOARD_COLUMNS;
    board.cells[rowEdgeEnd][colEdgeEnd] = PLAYER_O;
    test_assert(isValidMove(&board, rowEdgeEnd, colEdgeEnd, PLAYER_X) == FALSE,
                "Should not allow move to already marked position", results);

    test_end("ValidateInputFailedNotEmpty");
}



void testGetPlayerMoveDoesNotPrintDebugOutput(TestResults* results) {
    test_begin("GetPlayerMoveDoesNotPrintDebugOutput");

    Game game = initGame(PLAYER_PLAYER);

    char input[] = "5,5\n";
    FILE* stdin_backup = stdin;
    stdin = fmemopen(input, sizeof(input), "r");

    const char* path = "test_get_player_move_output.txt";
    FILE* fp = fopen(path, "w");
    FILE* stdout_backup = stdout;
    stdout = fp;
    Move move = getPlayerMove(&game);
    stdout = stdout_backup;
    fclose(fp);

    stdin = stdin_backup;

    char output[512] = "";
    fp = fopen(path, "r");
    if (fp) {
        size_t read = fread(output, 1, sizeof(output) - 1, fp);
        output[read] = '\0';
        fclose(fp);
    }
    remove(path);

    test_assert(move.row == 5 && move.col == 5,
                "Should return the entered move", results);
    test_assert(strstr(output, "row 5, col 5") == NULL,
                "Should not print debug output for the entered move", results);

    test_end("GetPlayerMoveDoesNotPrintDebugOutput");
}

// 空点が (4,9) だけで、そこは黒にとって長連になる禁じ手の局面
static void setUpBlackTrappedBoard(Game* game) {
    const char *trapped[] = {
            NULL,
            "OOOOOOOOX",
            "OOOOOOOOX",
            "OOOOOOOOX",
            "OOOOOOOO.",
            "OOOOOOOOX",
            "OOOOOOOOX",
            "OOOOOOOOX",
            "OOOOOOOOX",
            "OOOOOOOOX"
        };
    initBoardWithStr(&game->board, trapped);
    game->moveCount = 1;
    game->handHistory[0].move.row = 9;
    game->handHistory[0].move.col = BOARD_COLUMNS;
    game->handHistory[0].player = PLAYER_O;
}

void testBlackLosesWhenNoLegalMoveRemains(TestResults* results) {
    test_begin("BlackLosesWhenNoLegalMoveRemains");

    // docs/renju-rules.md の確定仕様:
    // 手番側に合法手が1つも無い場合はその手番側の負け。
    // 黒は禁点に打たされたものとみなし、白の勝ちになる。
    Game game = initGame(CPU_CPU);
    setUpBlackTrappedBoard(&game);
    game.currentPlayer = PLAYER_X;

    playTurn(&game);

    test_assert(game.gameState == GAME_WIN,
                "Game should end when black has no legal move", results);
    test_assert(game.winner == PLAYER_O,
                "White should win when black is forced onto a forbidden point", results);

    test_end("BlackLosesWhenNoLegalMoveRemains");
}

void testFullBoardIsDrawNotLoss(TestResults* results) {
    test_begin("FullBoardIsDrawNotLoss");

    // 満局は「合法手が無い」より先に引き分けと判定されること
    const char *full[] = {
            NULL,
            "OOOOOOOOX",
            "OOOOOOOOX",
            "OOOOOOOOX",
            "OOOOOOOOX",
            "OOOOOOOOX",
            "OOOOOOOOX",
            "OOOOOOOOX",
            "OOOOOOOOX",
            "OOOOOOOOX"
        };
    Game game = initGame(CPU_CPU);
    initBoardWithStr(&game.board, full);
    game.moveCount = 1;
    game.handHistory[0].move.row = 9;
    game.handHistory[0].move.col = BOARD_COLUMNS;
    game.handHistory[0].player = PLAYER_O;
    game.currentPlayer = PLAYER_X;

    playTurn(&game);

    test_assert(game.gameState == GAME_DRAW,
                "Full board should be a draw", results);

    test_end("FullBoardIsDrawNotLoss");
}

void runGameTests() {
    TestResults results = {0, 0, 0};
    test_suite_begin("Game Tests");
    
    suppress_output();

    testGameInitialization(&results);
    testPlayerSwitch(&results);
    testPlaceMoveExpected(&results);
    testPlaceMoveFailedAlreadyMarked(&results);
    testValidateInputExpectedRange(&results);
    testValidateInputFailedOutOfRange(&results);
    testValidateInputFailedNotEmpty(&results);
    testGetPlayerMoveDoesNotPrintDebugOutput(&results);
    testBlackLosesWhenNoLegalMoveRemains(&results);
    testFullBoardIsDrawNotLoss(&results);

    restore_output();

    test_suite_end("Game Tests", &results);
}

