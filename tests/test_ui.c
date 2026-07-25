#include <stdio.h>
#include <stdlib.h>
#include <string.h>
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

void testGetPlayerInputReturnsQuitOnEof(TestResults* results) {
    test_begin("GetPlayerInputReturnsQuitOnEof");

    // 入力が尽きた (EOF) 場合、プロンプトを出し続けるのではなく終了を返す
    FILE* stdin_backup = stdin;
    stdin = fopen("/dev/null", "r");

    Move move = getPlayerInput();

    test_assert(isQuitMove(move) == TRUE,
                "EOF should be reported as a quit move", results);

    fclose(stdin);
    stdin = stdin_backup;

    test_end("GetPlayerInputReturnsQuitOnEof");
}

void testGetPlayerInputReturnsQuitOnQ(TestResults* results) {
    test_begin("GetPlayerInputReturnsQuitOnQ");

    char input[] = "q\n";
    FILE* stdin_backup = stdin;
    stdin = fmemopen(input, sizeof(input), "r");

    Move move = getPlayerInput();

    test_assert(isQuitMove(move) == TRUE,
                "'q' should be reported as a quit move", results);

    stdin = stdin_backup;

    test_end("GetPlayerInputReturnsQuitOnQ");
}

void testAskForRematchReturnsFalseOnEof(TestResults* results) {
    test_begin("AskForRematchReturnsFalseOnEof");

    FILE* stdin_backup = stdin;
    stdin = fopen("/dev/null", "r");

    test_assert(askForRematch() == FALSE,
                "EOF should end the rematch loop", results);

    fclose(stdin);
    stdin = stdin_backup;

    test_end("AskForRematchReturnsFalseOnEof");
}

void testSelectGameModeReturnsQuitOnEof(TestResults* results) {
    test_begin("SelectGameModeReturnsQuitOnEof");

    FILE* stdin_backup = stdin;
    stdin = fopen("/dev/null", "r");

    test_assert(selectGameMode() == MODE_QUIT,
                "EOF should end the mode selection loop", results);

    fclose(stdin);
    stdin = stdin_backup;

    test_end("SelectGameModeReturnsQuitOnEof");
}

void runUiTests() {
    TestResults results = {0, 0, 0};
    test_suite_begin("UI Tests");
    
    suppress_output();

    testPrintBoard(&results);
    testGetPlayerInputExpectedStr(&results);
    testGetPlayerInputWithoutComma(&results);
    testGetPlayerInputWithSpace(&results);
    testGetPlayerInputWithoutSpace(&results);
    testGetPlayerInputReturnsQuitOnEof(&results);
    testGetPlayerInputReturnsQuitOnQ(&results);
    testAskForRematchReturnsFalseOnEof(&results);
    testSelectGameModeReturnsQuitOnEof(&results);

    restore_output();
    
    test_suite_end("UI Tests", &results);
}

