#include <stdio.h>
#include <assert.h>
#include "../include/board.h"
#include "../include/utils.h"
#include "test_utils.h"


static FILE* original_stdout = NULL;
static FILE* dev_null = NULL;

void test_begin(const char* test_name) {
    fprintf(stderr, "  [TEST] %s\n", test_name);
}

void test_end(const char* test_name) {
    fprintf(stderr, "  [PASS] %s\n", test_name);
}

void test_suite_begin(const char* suite_name) {
    fprintf(stderr, "[SUITE] %s\n", suite_name);
}

void test_suite_end(const char* suite_name, TestResults* results) {
    fprintf(stderr, "[DONE]  %s: %d passed, %d failed\n\n", 
            suite_name, results->passed, results->failed);
}

void test_assert(BOOL condition, const char* message, TestResults* results) {
    results->total++;
    if (condition) {
        results->passed++;
    } else {
        results->failed++;
        fprintf(stderr, "  [FAIL] %s\n", message);
    }
}

void suppress_output(void) {
    if (!original_stdout) {
        original_stdout = stdout;
#ifdef _WIN32
        dev_null = fopen("NUL", "w");
#else
        dev_null = fopen("/dev/null", "w");
#endif
        stdout = dev_null;
    }
}

void restore_output(void) {
    if (original_stdout) {
        stdout = original_stdout;
        fclose(dev_null);
        original_stdout = NULL;
        dev_null = NULL;
    }
}

static FILE* original_stderr = NULL;
static FILE* stderr_dev_null = NULL;

// 意図的な失敗を記録するテストで、[FAIL] 表示が混ざらないようにする
void suppress_stderr(void) {
    if (!original_stderr) {
        original_stderr = stderr;
#ifdef _WIN32
        stderr_dev_null = fopen("NUL", "w");
#else
        stderr_dev_null = fopen("/dev/null", "w");
#endif
        stderr = stderr_dev_null;
    }
}

void restore_stderr(void) {
    if (original_stderr) {
        stderr = original_stderr;
        fclose(stderr_dev_null);
        original_stderr = NULL;
        stderr_dev_null = NULL;
    }
}

int exitCodeFor(int failedCount) {
    return failedCount > 0 ? 1 : 0;
}


void testInitBoardWithStr(TestResults* results) {
    test_begin("InitBoardWithStr");
    
    Board board;
    const char *lines[] = {
        NULL,
        "........O",
        "...X.....",
        "..X......",
        "..X...O..",
        "X...O....",
        ".........",
        ".....@...",
        ".........",
        "........X"
    };
    
    initBoardWithStr(&board, lines);
    
    test_assert(board.cells[1][9] == PLAYER_O, "Position [1][9] should be PLAYER_O", results);
    test_assert(board.cells[2][4] == PLAYER_X, "Position [2][4] should be PLAYER_X", results);
    test_assert(board.cells[3][3] == PLAYER_X, "Position [3][3] should be PLAYER_X", results);
    test_assert(board.cells[4][3] == PLAYER_X, "Position [4][3] should be PLAYER_X", results);
    test_assert(board.cells[4][7] == PLAYER_O, "Position [4][7] should be PLAYER_O", results);
    test_assert(board.cells[5][1] == PLAYER_X, "Position [5][1] should be PLAYER_X", results);
    test_assert(board.cells[5][5] == PLAYER_O, "Position [5][5] should be PLAYER_O", results);
    test_assert(board.cells[7][6] == EMPTY_CELL, "Position [7][6] should be EMPTY_CELL", results);
    test_assert(board.cells[9][1] == EMPTY_CELL, "Position [9][1] should be EMPTY_CELL", results);
    test_assert(board.cells[9][9] == PLAYER_X, "Position [9][9] should be PLAYER_X", results);
    
    test_end("InitBoardWithStr");
}

void testMax(TestResults* results) {
    test_begin("Max");
    
    int x = 10, y = 3;
    test_assert(max(x, y) == x, "max(10, 3) should return 10", results);

    x = -19;
    y = 3;
    test_assert(max(x, y) == y, "max(-19, 3) should return 3", results);
    
    test_end("Max");
}

int runUtilsTests(void) {
    TestResults results = {0, 0, 0};
    test_suite_begin("Utils Tests");
    
    suppress_output();
    
    testInitBoardWithStr(&results);
    testMax(&results);
    
    restore_output();
    
    test_suite_end("Utils Tests", &results);
    return results.failed;
}
