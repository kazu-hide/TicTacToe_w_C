#include <stdio.h>
#include <assert.h>
#include "../include/utils.h"
#include "../include/board.h"
#include "../include/game.h"
#include "../include/cpu.h"
#include "test_utils.h"
#include "test_cpu.h"

void testEvaluateOpenFour(TestResults* results) {
    test_begin("EvaluateOpenFour");

    Board board = __prepareBoard();
    board.cells[2][2] = PLAYER_X;
    board.cells[2][3] = PLAYER_X;
    board.cells[2][4] = PLAYER_X;
    board.cells[2][5] = PLAYER_X;
    EvaluationScores scores = __evaluateStones(&board, PLAYER_X);
    
    test_assert(scores.lengthScore == OPEN_FOUR_POINTS, 
                "Open four should score OPEN_FOUR_POINTS", results);
    test_assert(scores.positionScore == 32, 
                "Position score should be 32", results);
    
    test_end("EvaluateOpenFour");
}

void testEvaluateCloseFour(TestResults* results) {
    test_begin("EvaluateCloseFour");

    Board board = __prepareBoard();
    board.cells[1][1] = PLAYER_X;
    board.cells[2][1] = PLAYER_X;
    board.cells[3][1] = PLAYER_X;
    board.cells[4][1] = PLAYER_X;
    EvaluationScores scores = __evaluateStones(&board, PLAYER_X);
    
    test_assert(scores.lengthScore == CLOSED_FOUR_POINTS, 
                "Close four should score CLOSED_FOUR_POINTS", results);
    test_assert(scores.positionScore == 18, 
                "Position score should be 18", results);
    
    test_end("EvaluateCloseFour");
}

void testEvaluateOpenThree(TestResults* results) {
    test_begin("EvaluateOpenThree");

    Board board = __prepareBoard();
    board.cells[4][4] = PLAYER_X;
    board.cells[5][3] = PLAYER_X;
    board.cells[6][2] = PLAYER_X;
    EvaluationScores scores = __evaluateStones(&board, PLAYER_X);

    test_assert(scores.lengthScore == OPEN_THREE_POINTS, 
                "Open three should score OPEN_THREE_POINTS", results);
    test_assert(scores.positionScore == 40, 
                "Position score should be 40", results);
    
    test_end("EvaluateOpenThree");
}

void testEvaluateCloseThree(TestResults* results) {
    test_begin("EvaluateCloseThree");

    Board board = __prepareBoard();
    board.cells[7][7] = PLAYER_X;
    board.cells[8][8] = PLAYER_X;
    board.cells[9][9] = PLAYER_X;
    EvaluationScores scores = __evaluateStones(&board, PLAYER_X);

    test_assert(scores.lengthScore == CLOSED_THREE_POINTS, 
                "Close three should score CLOSED_THREE_POINTS", results);
    test_assert(scores.positionScore == 19, 
                "Position score should be 19", results);
    
    test_end("EvaluateCloseThree");
}

void testEvaluateOpenTwo(TestResults* results) {
    test_begin("EvaluateOpenTwo");

    Board board = __prepareBoard();
    board.cells[4][9] = PLAYER_X;
    board.cells[5][9] = PLAYER_X;
    EvaluationScores scores = __evaluateStones(&board, PLAYER_X);

    test_assert(scores.lengthScore == OPEN_TWO_POINTS, 
                "Open two should score OPEN_TWO_POINTS", results);
    test_assert(scores.positionScore == 10, 
                "Position score should be 10", results);
    
    test_end("EvaluateOpenTwo");
}

void testEvaluateCloseTwo(TestResults* results) {
    test_begin("EvaluateCloseTwo");

    Board board = __prepareBoard();
    board.cells[9][1] = PLAYER_X;
    board.cells[9][2] = PLAYER_X;
    EvaluationScores scores = __evaluateStones(&board, PLAYER_X);

    test_assert(scores.lengthScore == CLOSED_TWO_POINTS, 
                "Close two should score CLOSED_TWO_POINTS", results);
    test_assert(scores.positionScore == 8, 
                "Position score should be 8", results);
    
    test_end("EvaluateCloseTwo");
}

void testEvaluateDoubleClosedLine(TestResults* results) {
    test_begin("EvaluateDoubleClosedLine");

    Board board = __prepareBoard();
    board.cells[1][1] = PLAYER_X;
    board.cells[1][2] = PLAYER_X;
    board.cells[1][3] = PLAYER_X;
    board.cells[1][4] = PLAYER_X;
    board.cells[1][5] = PLAYER_O;
    EvaluationScores scores = __evaluateStones(&board, PLAYER_X);

    test_assert(scores.lengthScore == 0, 
                "Double closed line should score 0", results);
    test_assert(scores.positionScore == 17, 
                "Position score should be 17", results);
    
    test_end("EvaluateDoubleClosedLine");
}

void testEvaluateMultiLine(TestResults* results) {
    test_begin("EvaluateMultiLine");

    Board board = __prepareBoard();
    board.cells[1][6] = PLAYER_X;
    board.cells[1][7] = PLAYER_X;
    board.cells[1][8] = PLAYER_X;
    board.cells[1][9] = PLAYER_X;
    board.cells[2][7] = PLAYER_X;
    board.cells[3][7] = PLAYER_X;
    board.cells[4][7] = PLAYER_X;
    EvaluationScores scores = __evaluateStones(&board, PLAYER_X);

    test_assert(scores.lengthScore == CLOSED_FOUR_POINTS * 2 + CLOSED_TWO_POINTS * 4,
                "Multi line should score CLOSED_FOUR_POINTS * 2 + CLOSED_TWO_POINTS * 4", results);
    test_assert(scores.positionScore == 42, 
                "Position score should be 42", results);
    
    test_end("EvaluateMultiLine");
}

void testNegaMax(TestResults* results) {
    test_begin("NegaMax");

    Board board = __prepareBoard();
    board.cells[2][2] = PLAYER_X;
    board.cells[2][3] = PLAYER_X;
    board.cells[2][4] = PLAYER_X;
    board.cells[2][5] = PLAYER_X;
    board.cells[7][9] = PLAYER_O;
    board.cells[8][9] = PLAYER_O;

    int bestRow = -1, bestCol = -1;
    negaMax(&board, NEGA_MAX_DEPTH, PLAYER_X, 0, 0, &bestRow, &bestCol,
            -SCORE_INFINITY, SCORE_INFINITY);

    // (2,2)-(2,5) の四に対し、(2,1) と (2,6) はどちらも即座に五になる等価な手。
    // 特定の列ではなく「五を作る手を選ぶこと」を検証する。
    test_assert(bestRow == 2,
                "NegaMax should choose row 2 for best move", results);
    test_assert(isWinMove(&board, bestRow, bestCol, PLAYER_X) == TRUE,
                "NegaMax should choose a winning move", results);
    
    test_end("NegaMax");
}

void testGetCpuMoveReturnsMoveInBoard(TestResults* results) {
    test_begin("GetCpuMoveReturnsMoveInBoard");

    // (1,1)-(4,4) に手番側の石が並んでいる局面。
    // 終端判定が「直前の手」ではなく初期値 (0,0) を見ていると、
    // isWinMove(board, 0, 0, X) が対角線を数え上げて探索前に打ち切られ、
    // 手が一度も選ばれないまま (0,0) が返ってしまう。
    Game game = initGame(CPU_CPU);
    game.board.cells[1][1] = PLAYER_X;
    game.board.cells[2][2] = PLAYER_X;
    game.board.cells[3][3] = PLAYER_X;
    game.board.cells[4][4] = PLAYER_X;
    game.board.cells[8][1] = PLAYER_O;
    game.board.cells[8][2] = PLAYER_O;
    game.board.cells[9][1] = PLAYER_O;
    game.board.cells[9][2] = PLAYER_O;
    game.currentPlayer = PLAYER_X;

    Move move = getCpuMove(&game);

    test_assert(isInBoard(move.row, move.col) == TRUE,
                "CPU move should be inside the board", results);
    test_assert(game.board.cells[move.row][move.col] == EMPTY_CELL,
                "CPU move should be an empty cell", results);

    test_end("GetCpuMoveReturnsMoveInBoard");
}

void testGetCpuMoveTakesImmediateWin(TestResults* results) {
    test_begin("GetCpuMoveTakesImmediateWin");

    // O は (5,2)-(5,5) の四。(5,1) または (5,6) に打てば即座に五。
    Game game = initGame(PLAYER_CPU);
    game.board.cells[5][2] = PLAYER_O;
    game.board.cells[5][3] = PLAYER_O;
    game.board.cells[5][4] = PLAYER_O;
    game.board.cells[5][5] = PLAYER_O;
    game.board.cells[1][1] = PLAYER_X;
    game.board.cells[9][9] = PLAYER_X;
    game.currentPlayer = PLAYER_O;

    Move move = getCpuMove(&game);

    test_assert(isWinMove(&game.board, move.row, move.col, PLAYER_O) == TRUE,
                "CPU should play the immediately winning move", results);

    test_end("GetCpuMoveTakesImmediateWin");
}

void testGetCpuMoveBlocksImmediateLoss(TestResults* results) {
    test_begin("GetCpuMoveBlocksImmediateLoss");

    // X が (5,2)-(5,5) の四。左端 (5,1) は O が塞いでいるため、
    // 五を止められる手は (5,6) の一択。
    Game game = initGame(PLAYER_CPU);
    game.board.cells[5][2] = PLAYER_X;
    game.board.cells[5][3] = PLAYER_X;
    game.board.cells[5][4] = PLAYER_X;
    game.board.cells[5][5] = PLAYER_X;
    game.board.cells[5][1] = PLAYER_O;
    game.board.cells[9][9] = PLAYER_O;
    game.currentPlayer = PLAYER_O;

    Move move = getCpuMove(&game);

    test_assert((move.row == 5 && move.col == 6) == TRUE,
                "CPU should block the opponent's four", results);

    test_end("GetCpuMoveBlocksImmediateLoss");
}

void testIsGameOverRejectsOutOfBoardCell(TestResults* results) {
    test_begin("IsGameOverRejectsOutOfBoardCell");

    // (0,0) は盤外。盤上の対角線を数えて「勝ち」と判定してはいけない。
    Board board = __prepareBoard();
    board.cells[1][1] = PLAYER_X;
    board.cells[2][2] = PLAYER_X;
    board.cells[3][3] = PLAYER_X;
    board.cells[4][4] = PLAYER_X;

    test_assert(isGameOver(&board, 0, 0, PLAYER_X) == FALSE,
                "Out of board cell should never end the game", results);

    test_end("IsGameOverRejectsOutOfBoardCell");
}

void testNegaMaxScoresNoLegalMoveAsLoss(TestResults* results) {
    test_begin("NegaMaxScoresNoLegalMoveAsLoss");

    // 空点が (4,9) だけで、そこは黒にとって長連になる禁じ手。
    // 黒番のこのノードは負けが確定しているので、決定的な負けスコアを返すこと。
    // (docs/renju-rules.md の確定仕様)
    Board board;
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
    initBoardWithStr(&board, trapped);

    int bestRow = -1, bestCol = -1;
    int score = negaMax(&board, 1, PLAYER_X, 0, 0, &bestRow, &bestCol,
                        -SCORE_INFINITY, SCORE_INFINITY);

    test_assert(score == -(TERMINAL_WIN_SCORE + 1),
                "A node with no legal move should score as a loss", results);

    test_end("NegaMaxScoresNoLegalMoveAsLoss");
}

int runCPUTests(void) {
    TestResults results = {0, 0, 0};
    test_suite_begin("CPU Tests");
    
    suppress_output();
    
    testEvaluateOpenFour(&results);
    testEvaluateCloseFour(&results);
    testEvaluateOpenThree(&results);
    testEvaluateCloseThree(&results);
    testEvaluateOpenTwo(&results);
    testEvaluateCloseTwo(&results);
    testEvaluateDoubleClosedLine(&results);
    testEvaluateMultiLine(&results);
    testNegaMax(&results);
    testGetCpuMoveReturnsMoveInBoard(&results);
    testGetCpuMoveTakesImmediateWin(&results);
    testGetCpuMoveBlocksImmediateLoss(&results);
    testIsGameOverRejectsOutOfBoardCell(&results);
    testNegaMaxScoresNoLegalMoveAsLoss(&results);

    restore_output();
    
    test_suite_end("CPU Tests", &results);
    return results.failed;
}
