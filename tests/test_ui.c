#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/board.h"
#include "../include/ui.h"
#include "../include/game.h"
#include "test_utils.h"
#include "test_ui.h"

void testPrintBoard(TestResults* results) {
    test_begin("PrintBoard");

    Game game = initGame(PLAYER_PLAYER);

    game.board.cells[1][1] = PLAYER_X;
    game.board.cells[5][7] = PLAYER_O;
    game.board.cells[9][9] = PLAYER_X;

    game.moveCount = 3;
    game.handHistory[2].move.row = 9;
    game.handHistory[2].move.col = 9;
    game.handHistory[2].player = PLAYER_X;

    FILE* fp = fopen("test_print_board_output.txt", "w");
    FILE* stdout_backup = stdout;
    stdout = fp;
    printBoard(&game);
    stdout = stdout_backup;
    fclose(fp);

    fp = fopen("test_print_board_output.txt", "r");
    if (!fp) {
        test_assert(FALSE, "Failed to open test output file", results);
        test_end("PrintBoard");
        return;
    }

    const char *expected[] = {
        "\t1   2   3   4   5   6   7   8   9   \n",
        "\n",
        "1\tX |   |   |   |   |   |   |   |  \n",
        "\t- + - + - + - + - + - + - + - + -\n",
        "2\t  |   |   |   |   |   |   |   |  \n",
        "\t- + - + - + - + - + - + - + - + -\n",
        "3\t  |   |   |   |   |   |   |   |  \n",
        "\t- + - + - + - + - + - + - + - + -\n",
        "4\t  |   |   |   |   |   |   |   |  \n",
        "\t- + - + - + - + - + - + - + - + -\n",
        "5\t  |   |   |   |   |   | O |   |  \n",
        "\t- + - + - + - + - + - + - + - + -\n",
        "6\t  |   |   |   |   |   |   |   |  \n",
        "\t- + - + - + - + - + - + - + - + -\n",
        "7\t  |   |   |   |   |   |   |   |  \n",
        "\t- + - + - + - + - + - + - + - + -\n",
        "8\t  |   |   |   |   |   |   |   |  \n",
        "\t- + - + - + - + - + - + - + - + -\n",
        "9\t  |   |   |   |   |   |   |   | \x1b[32mX\x1b[39m\n", 
        "\n", 
        NULL
    };

    char buffer[100];
    int line = 0;
    while(fgets(buffer, sizeof(buffer), fp) != NULL) {
        if (expected[line] == NULL) break;
        test_assert(strcmp(buffer, expected[line]) == 0,
                   "Board output should match expected format", results);
        line++;
    }

    fclose(fp);
    remove("test_print_board_output.txt");
    
    test_end("PrintBoard");
}

// 出力から "<row>\t" で始まる行を取り出す
static void readBoardRow(const char* path, int row, char* out, size_t size) {
    char prefix[8];
    snprintf(prefix, sizeof(prefix), "%d\t", row);

    out[0] = '\0';
    FILE* fp = fopen(path, "r");
    if (!fp)
        return;

    char buffer[256];
    while (fgets(buffer, sizeof(buffer), fp) != NULL) {
        if (strncmp(buffer, prefix, strlen(prefix)) == 0) {
            snprintf(out, size, "%s", buffer);
            break;
        }
    }
    fclose(fp);
}

void testPrintBoardProhibitedMoveInLastColumn(TestResults* results) {
    test_begin("PrintBoardProhibitedMoveInLastColumn");

    // 9列目の 1,2,3,5,6 行目に X。(4,9) に打つと長連 (6連) になるため禁じ手。
    Game game = initGame(PLAYER_PLAYER);
    game.board.cells[1][BOARD_COLUMNS] = PLAYER_X;
    game.board.cells[2][BOARD_COLUMNS] = PLAYER_X;
    game.board.cells[3][BOARD_COLUMNS] = PLAYER_X;
    game.board.cells[5][BOARD_COLUMNS] = PLAYER_X;
    game.board.cells[6][BOARD_COLUMNS] = PLAYER_X;

    // printBoard() は「次の手番」の禁じ手を表示する
    game.currentPlayer = PLAYER_O;
    game.moveCount = 1;
    game.handHistory[0].move.row = 6;
    game.handHistory[0].move.col = BOARD_COLUMNS;
    game.handHistory[0].player = PLAYER_X;

    test_assert(isProhibitedMove(&game.board, 4, BOARD_COLUMNS, PLAYER_X) == TRUE,
                "Test setup: (4, 9) should be a prohibited move", results);

    const char* path = "test_print_board_last_column.txt";
    FILE* fp = fopen(path, "w");
    FILE* stdout_backup = stdout;
    stdout = fp;
    printBoard(&game);
    stdout = stdout_backup;
    fclose(fp);

    char row4[256];
    readBoardRow(path, 4, row4, sizeof(row4));
    remove(path);

    // 4行目の最終列が赤い * で表示されていること
    test_assert(strstr(row4, "\x1b[31m*\x1b[39m\n") != NULL,
                "Prohibited move in the last column should be marked", results);

    test_end("PrintBoardProhibitedMoveInLastColumn");
}

void testGetPlayerInputExpectedStr(TestResults* results) {
    test_begin("GetPlayerInputExpectedStr");

    char input[] = "1, 2\n";
    FILE* stdin_backup = stdin;
    stdin = fmemopen(input, sizeof(input), "r");
    Move move = getPlayerInput();

    test_assert(move.row == 1 && move.col == 2,
                "Should correctly parse coordinates", results);

    stdin = stdin_backup;
    
    test_end("GetPlayerInputExpectedStr");
}

void testGetPlayerInputWithoutComma(TestResults* results) {
    test_begin("GetPlayerInputWithoutComma");

    char input[] = "1 2\n";
    FILE* stdin_backup = stdin;
    
    stdin = fmemopen(input, sizeof(input), "r");
    Move move = getPlayerInput();

    test_assert(move.row == 1 && move.col == 2,
                "Should correctly parse coordinates", results);

    stdin = stdin_backup;
    
    test_end("GetPlayerInputWithoutComma");
}

void testGetPlayerInputWithSpace(TestResults* results) {
    test_begin("GetPlayerInputWithSpace");

    char input[] = "1 , 2 \n";
    FILE* stdin_backup = stdin;
    
    stdin = fmemopen(input, sizeof(input), "r");
    Move move = getPlayerInput();

    test_assert(move.row == 1 && move.col == 2,
                "Should correctly parse coordinates", results);

    stdin = stdin_backup;
    
    test_end("GetPlayerInputWithSpace");
}

void testGetPlayerInputWithoutSpace(TestResults* results) {
    test_begin("GetPlayerInputWithoutSpace");

    char input[] = "1,2\n";
    FILE* stdin_backup = stdin;
    
    stdin = fmemopen(input, sizeof(input), "r");
    Move move = getPlayerInput();

    test_assert(move.row == 1 && move.col == 2,
                "Should correctly parse coordinates", results);

    stdin = stdin_backup;
    
    test_end("GetPlayerInputWithoutSpace");
}

void runUiTests() {
    TestResults results = {0, 0, 0};
    test_suite_begin("UI Tests");
    
    suppress_output();

    testPrintBoard(&results);
    testPrintBoardProhibitedMoveInLastColumn(&results);
    testGetPlayerInputExpectedStr(&results);
    testGetPlayerInputWithoutComma(&results);
    testGetPlayerInputWithSpace(&results);
    testGetPlayerInputWithoutSpace(&results);
    
    restore_output();
    
    test_suite_end("UI Tests", &results);
}

