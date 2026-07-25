#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/board.h"
#include "test_utils.h"
#include "test_board.h"

void testInitBoard(TestResults* results) {
    test_begin("InitBoard");

    Board board;
    board.cells[1][1] = PLAYER_X;
    board.cells[4][6] = PLAYER_O;
    board.cells[8][8] = PLAYER_X;
    
    initBoard(&board);

    for (int i = 1; i <= BOARD_ROWS; i++) {
        for (int j = 1; j <= BOARD_COLUMNS; j++) {
            test_assert(board.cells[i][j] == EMPTY_CELL,
                       "All cells should be empty after initialization", results);
        }
    }

    test_end("InitBoard");
}

void testIsWinMove(TestResults* results) {
    test_begin("IsWinMove");

    Board board;

    // 横に5つ並んでいる
    const char *testBoard1[] = {
        NULL,
        ".........",
        "..XXXX@..",
        ".........",
        ".........",
        ".........",
        ".........",
        ".........",
        ".........",
        "O@OOO...."
    };
    
    initBoardWithStr(&board, testBoard1);
    test_assert(isWinMove(&board, 2, 7, PLAYER_X) == TRUE,
                "Horizontal win pattern should be detected", results);
    test_assert(isWinMove(&board, 9, 2, PLAYER_O) == TRUE,
                "Horizontal win pattern for PLAYER_O should be detected", results);

    // 左上隅、FALSE case & TRUE case
    const char *testBoard2[] = {
        NULL,
        "@XXXX....",
        "O........",
        "O........",
        ".........",
        "O........",
        ".........",
        ".........",
        ".........",
        "........."
    };
    
    initBoardWithStr(&board, testBoard2);
    test_assert(isWinMove(&board, 1, 1, PLAYER_O) == FALSE,
                "Should not detect win for PLAYER_O", results);
    test_assert(isWinMove(&board, 1, 1, PLAYER_X) == TRUE,
                "Should detect win for PLAYER_X", results);

    // 長さが5を超えるケース
    const char *testBoard3[] = {
        NULL,
        ".........",
        "X........",
        "X........",
        "X..O.....",
        "@...O....",
        "X....O...",
        "X.....O..",
        ".......O.",
        "........@"
    };
    
    initBoardWithStr(&board, testBoard3);
    test_assert(isWinMove(&board, 5, 1, PLAYER_X) == FALSE,
                "Should not detect win for PLAYER_X", results);
    test_assert(isWinMove(&board, 9, 9, PLAYER_O) == TRUE,
                "Should detect win for PLAYER_O", results);

    //
    const char *testBoard4[] = {
        NULL,
        ".........",
        "......X..",
        ".....@...",
        "....X....",
        "...X.....",
        "..X......",
        ".........",
        ".........",
        "........."
    };
    
    initBoardWithStr(&board, testBoard4);
    test_assert(isWinMove(&board, 3, 6, PLAYER_X) == TRUE,
                "Should detect win for PLAYER_X", results);

    test_end("IsWinMove");
}

void testIsMakingOverLine(TestResults* results) {
    test_begin("IsMakingOverLine");

    Board board;
    const char *testBoard1[] = {
        NULL,
        ".........",
        ".XXXXX@..",
        ".........",
        ".........",
        ".........",
        ".........",
        ".........",
        ".........",
        "O@OOOO..."
    };
    
    initBoardWithStr(&board, testBoard1);
    test_assert(isMakingOverLine(&board, 2, 7, PLAYER_X) == TRUE,
                "Should detect overline for PLAYER_X", results);
    test_assert(isMakingOverLine(&board, 9, 2, PLAYER_O) == FALSE,
                "Should not detect overline for PLAYER_O", results);

    const char *testBoard2[] = {
        NULL,
        ".........",
        "X........",
        "X........",
        "X..X.....",
        "@...X....",
        "X....X...",
        "X.....X..",
        ".......X.",
        "........@"
    };
    
    initBoardWithStr(&board, testBoard2);
    test_assert(isMakingOverLine(&board, 5, 1, PLAYER_X) == TRUE,
                "Should detect overline for PLAYER_X", results);
    test_assert(isMakingOverLine(&board, 9, 9, PLAYER_X) == TRUE,
                "Should detect overline for PLAYER_X", results);

    test_end("IsMakingOverLine");
}

void testCountContinuousStonesWithGap(TestResults* results) {
    test_begin("CountContinuousStonesWithGap");

    Board board;
    
    // テストケース1: 左右方向に2種類のラインができるケース
    const char *testBoard1[] = {
        NULL,
        ".........",
        "X.X@.X...",
        ".........",
        ".........",
        ".........",
        ".........",
        ".........",
        ".........",
        "........."
    };
    
    initBoardWithStr(&board, testBoard1);
    LinePatterns result1 = countContinuousStonesWithGap(&board, 2, 4, 0, 1, PLAYER_X);
    test_assert(result1.pattern == 2,
                "Should detect 2 continuous stones with gap", results);
    test_assert(result1.lines[0].start.r == 2,
                "Should detect correct start row for line 1", results);
    test_assert(result1.lines[0].start.c == 1,
                "Should detect correct start column for line 1", results);
    test_assert(result1.lines[0].end.r == 2,
                "Should detect correct end row for line 1", results);
    test_assert(result1.lines[0].end.c == 4,
                "Should detect correct end column for line 1", results);
    test_assert(result1.lines[0].dir.dx == 0,
                "Should detect correct dx for line 1", results);
    test_assert(result1.lines[0].dir.dy == 1,
                "Should detect correct dy for line 1", results);
    test_assert(result1.lines[1].start.r == 2,
                "Should detect correct start row for line 2", results);
    test_assert(result1.lines[1].start.c == 3,
                "Should detect correct start column for line 2", results);
    test_assert(result1.lines[1].end.r == 2,
                "Should detect correct end row for line 2", results);
    test_assert(result1.lines[1].end.c == 6,
                "Should detect correct end column for line 2", results);
    

    // テストケース2: 1種類のみのケース (gapなし)
    const char *testBoard2[] = {
        NULL,
        ".........",
        "..X@X....",
        ".........",
        ".........",
        ".........",
        ".........",
        ".........",
        ".........",
        "........."
    };
    
    initBoardWithStr(&board, testBoard2);
    LinePatterns result2 = countContinuousStonesWithGap(&board, 2, 4, 0, 1, PLAYER_X);
    test_assert(result2.pattern == 1,
                "Should detect 1 continuous stone with gap", results);
    test_assert(result2.lines[0].start.r == 2,
                "Should detect correct start row for line 1", results);
    test_assert(result2.lines[0].start.c == 3,
                "Should detect correct start column for line 1", results);
    test_assert(result2.lines[0].end.r == 2,
                "Should detect correct end row for line 1", results);
    test_assert(result2.lines[0].end.c == 5,
                "Should detect correct end column for line 1", results);
    test_assert(result2.lines[0].dir.dx == 0,
                "Should detect correct dx for line 1", results);
    test_assert(result2.lines[0].dir.dy == 1,
                "Should detect correct dy for line 1", results);
    

        // テストケース3: 片側にしかgapがないケース
    const char *testBoard3[] = {
        NULL,
        ".........",
        "XXX@.X..",
        ".........",
        ".........",
        ".........",
        ".........",
        ".........",
        ".........",
        "........."
    };
    
    initBoardWithStr(&board, testBoard3);
    LinePatterns result3 = countContinuousStonesWithGap(&board, 2, 4, 0, 1, PLAYER_X);
    test_assert(result3.pattern == 1,
                "Should detect 1 continuous stone with gap", results);
    test_assert(result3.lines[0].start.r == 2,
                "Should detect correct start row for line 1", results);
    test_assert(result3.lines[0].start.c == 1,
                "Should detect correct start column for line 1", results);
    test_assert(result3.lines[0].end.r == 2,
                "Should detect correct end row for line 1", results);
    test_assert(result3.lines[0].end.c == 6,
                "Should detect correct end column for line 1", results);
    test_assert(result3.lines[0].dir.dx == 0,
                "Should detect correct dx for line 1", results);
    test_assert(result3.lines[0].dir.dy == 1,
                "Should detect correct dy for line 1", results);
    

    test_end("CountContinuousStonesWithGap");
}

void testIsOpenLine(TestResults* results) {
    test_begin("IsOpenLine");

    Board board;

    const char *testBoard1[] = {
            NULL,
            ".........",
            "..XXX....", 
            ".......X.",
            "..X...X..",
            "..X......",
            ".....X...",
            "......X..",
            ".......X.",
            "........."
        };
    initBoardWithStr(&board, testBoard1);
    Cell start1 = {.r = 2, .c = 3};
    Cell end1 = {.r = 2, .c = 5};
    Direction dir1 = {.dx = 0, .dy = 1};

    Cell start2 = {.r = 4, .c = 3};
    Cell end2 = {.r = 5, .c = 3};
    Direction dir2 = {.dx = 1, .dy = 0};

    Cell start3 = {.r = 3, .c = 8};
    Cell end3 = {.r = 4, .c = 7};
    Direction dir3 = {.dx = 1, .dy = -1};

    Cell start4 = {.r = 6, .c = 6};
    Cell end4 = {.r = 8, .c = 8};
    Direction dir4 = {.dx = 1, .dy = 1};

    test_assert(isOpenLine(&board, start1, end1, dir1) == TRUE,
                "Should detect open line", results);
    test_assert(isOpenLine(&board, start2, end2, dir2) == TRUE,
                "Should detect open line", results);
    test_assert(isOpenLine(&board, start3, end3, dir3) == TRUE,
                "Should detect open line", results);
    test_assert(isOpenLine(&board, start4, end4, dir4) == TRUE,
                "Should detect open line", results);


    const char *testBoard2[] = {
            NULL,
            ".........",
            "XXXX....X", 
            ".......X.",
            "..X...X..",
            "..X......",
            "..O..X...",
            "......X..",
            ".......X.",
            "........O"
        };
    initBoardWithStr(&board, testBoard2);
    Cell start5 = {.r = 2, .c = 1};
    Cell end5 = {.r = 2, .c = 4};
    Direction dir5 = {.dx = 0, .dy = 1};

    Cell start6 = {.r = 4, .c = 3};
    Cell end6 = {.r = 5, .c = 3};
    Direction dir6 = {.dx = 1, .dy = 0};

    Cell start7 = {.r = 2, .c = 9};
    Cell end7 = {.r = 4, .c = 7};
    Direction dir7 = {.dx = 1, .dy = -1};

    Cell start8 = {.r = 6, .c = 6};
    Cell end8 = {.r = 8, .c = 8};
    Direction dir8 = {.dx = 1, .dy = 1};

    test_assert(isOpenLine(&board, start5, end5, dir5) == FALSE,
                "Should not detect open line", results);
    test_assert(isOpenLine(&board, start6, end6, dir6) == FALSE,
                "Should not detect open line", results);
    test_assert(isOpenLine(&board, start7, end7, dir7) == FALSE,
                "Should not detect open line", results);
    test_assert(isOpenLine(&board, start8, end8, dir8) == FALSE,
                "Should not detect open line", results);

    test_end("IsOpenLine");
}

void testIsHalfOpenLine(TestResults* results) {
    test_begin("IsHalfOpenLine");

    Board board;

    const char *testBoard1[] = {
            NULL,
            ".........",
            ".OXXX...X", 
            "..O....X.",
            "..X...X..",
            "..X......",
            "..O..X...",
            "......X..",
            ".......X.",
            "........."
        };
    initBoardWithStr(&board, testBoard1);
    Cell start1 = {.r = 2, .c = 3};
    Cell end1 = {.r = 2, .c = 5};
    Direction dir1 = {.dx = 0, .dy = 1};

    Cell start2 = {.r = 4, .c = 3};
    Cell end2 = {.r = 5, .c = 3};
    Direction dir2 = {.dx = 1, .dy = 0};

    Cell start3 = {.r = 2, .c = 9};
    Cell end3 = {.r = 4, .c = 7};
    Direction dir3 = {.dx = 1, .dy = -1};

    Cell start4 = {.r = 6, .c = 6};
    Cell end4 = {.r = 8, .c = 8};
    Direction dir4 = {.dx = 1, .dy = 1};

    test_assert(isAtLeastHalfOpenLine(&board, start1, end1, dir1) == TRUE,
                "Should detect half open line", results);
    test_assert(isAtLeastHalfOpenLine(&board, start2, end2, dir2) == FALSE,
                "Should not detect half open line", results);
    test_assert(isAtLeastHalfOpenLine(&board, start3, end3, dir3) == TRUE,
                "Should detect half open line", results);
    test_assert(isAtLeastHalfOpenLine(&board, start4, end4, dir4) == TRUE,
                "Should detect half open line", results);

    test_end("IsHalfOpenLine");
}

void testIsHalfOpenLineAtBoardEdge(TestResults* results) {
    test_begin("IsHalfOpenLineAtBoardEdge");

    // 盤の上端に接するライン。上側 (0行目) は盤外、下側 (5,9) は O が塞いでいるので
    // 「少なくとも片側が空いている」は成立しない。
    Board board;
    const char *testBoard[] = {
            NULL,
            "........X",
            "........X",
            "........X",
            "........X",
            "........O",
            ".........",
            ".........",
            ".........",
            "........."
        };
    initBoardWithStr(&board, testBoard);

    // initBoard() は 1..BOARD_ROWS しか初期化しないため 0 行目は不定値になる。
    // 盤外セルを参照していないことを確実に検証するため、空きマスに見える値を入れておく。
    for (int c = 0; c <= BOARD_COLUMNS; c++) {
        board.cells[0][c] = EMPTY_CELL;
    }

    Cell topStart = {.r = 1, .c = 9};
    Cell topEnd = {.r = 4, .c = 9};
    Direction dir = {.dx = 1, .dy = 0};

    test_assert(isAtLeastHalfOpenLine(&board, topStart, topEnd, dir) == FALSE,
                "Cell outside the board should not count as an open end", results);

    // 盤の下端に接するライン。下側は 10 行目で cells 配列の範囲外にあたる。
    const char *bottomBoard[] = {
            NULL,
            ".........",
            ".........",
            ".........",
            ".........",
            "........O",
            "........X",
            "........X",
            "........X",
            "........X"
        };
    initBoardWithStr(&board, bottomBoard);

    Cell bottomStart = {.r = 6, .c = 9};
    Cell bottomEnd = {.r = 9, .c = 9};

    test_assert(isAtLeastHalfOpenLine(&board, bottomStart, bottomEnd, dir) == FALSE,
                "Cell past the last row should not count as an open end", results);

    test_end("IsHalfOpenLineAtBoardEdge");
}

void testGetGapIdx(TestResults* results) {
    test_begin("GetGapIdx");

    Board board;
    
    // テストケース1: 水平方向のgap
    const char *testBoard1[] = {
        NULL,
        ".........",
        ".@XX.X...", 
        ".........",
        ".........",
        ".........",
        ".........",
        ".........",
        ".........",
        "........."
    };
    
    initBoardWithStr(&board, testBoard1);
    LineInfo line1 = {
        .start = {.r = 2, .c = 2},
        .end = {.r = 2, .c = 6},
        .dir = {.dx = 0, .dy = 1},
        .hasGap = TRUE
    };
    
    Cell gap1 = getGapIdx(&board, &line1);
    test_assert(gap1.r == 2,
                "Should detect correct row for gap", results);
    test_assert(gap1.c == 5,
                "Should detect correct column for gap", results);
    
    // テストケース2: gapなしのライン
    const char *testBoard2[] = {
        NULL,
        ".........",
        "..XXX....", 
        ".........",
        ".........",
        ".........",
        ".........",
        ".........",
        ".........",
        "........."
    };
    
    initBoardWithStr(&board, testBoard2);
    LineInfo line2 = {
        .start = {.r = 2, .c = 3},
        .end = {.r = 2, .c = 6},
        .dir = {.dx = 0, .dy = 1},
        .hasGap = FALSE
    };
    
    Cell gap2 = getGapIdx(&board, &line2);
    test_assert(gap2.r == -1,
                "Should not detect gap", results);
    test_assert(gap2.c == -1,
                "Should not detect gap", results);
    
    // テストケース3: 斜め方向のgap
    const char *testBoard3[] = {
        NULL,
        ".........",
        "..X......",
        "...X.....",
        ".........", 
        ".....X...",
        ".........",
        ".........",
        ".........",
        "........."
    };
    
    initBoardWithStr(&board, testBoard3);
    LineInfo line3 = {
        .start = {.r = 2, .c = 3},
        .end = {.r = 5, .c = 6},
        .dir = {.dx = 1, .dy = 1},
        .hasGap = TRUE
    };
    
    Cell gap3 = getGapIdx(&board, &line3);
    test_assert(gap3.r == 4,
                "Should detect correct row for gap", results);
    test_assert(gap3.c == 5,
                "Should detect correct column for gap", results);
    
    test_end("GetGapIdx");
}

void testIsFour(TestResults* results) {
    test_begin("IsFour");

    Board board;
    
    // テストケース1: gapあり、なしの四
    const char *testBoard1[] = {
        NULL,
        ".........",
        ".XXX.X...",
        ".........",
        "..X......",
        "...X.....",
        "....X....",
        ".....X...",
        ".........",
        "........."
    };
    
    initBoardWithStr(&board, testBoard1);
    LineInfo line1 = {
        .start = {.r = 2, .c = 2},
        .end = {.r = 2, .c = 6},
        .dir = {.dx = 0, .dy = 1},
        .hasGap = TRUE,
        .length = 4
    };

    LineInfo line2 = {
        .start = {.r = 4, .c = 3},
        .end = {.r = 7, .c = 6},
        .dir = {.dx = 1, .dy = 1},
        .hasGap = FALSE,
        .length = 4
    };
    test_assert(isFour(&board, &line1, PLAYER_X) == TRUE,
                "Should detect four", results);
    test_assert(isFour(&board, &line2, PLAYER_X) == TRUE,
                "Should detect four", results);

    // テストケース2: 四ではないケース
    const char *testBoard2[] = {
        NULL,
        ".........",
        ".XXOX....",
        ".........",
        "....XXX..",
        ".........",
        ".........",
        ".........",
        "XXXX.X...",
        "........."
    };
    
    initBoardWithStr(&board, testBoard2);
    LineInfo line3 = {
        .start = {.r = 2, .c = 2},
        .end = {.r = 2, .c = 6},
        .dir = {.dx = 0, .dy = 1},
        .hasGap = TRUE,
        .length = 4
    };

    LineInfo line4 = {
        .start = {.r = 4, .c = 5},
        .end = {.r = 4, .c = 7},
        .dir = {.dx = 0, .dy = 1},
        .hasGap = FALSE,
        .length = 3
    };

    LineInfo line5 = {
        .start = {.r = 8, .c = 1},
        .end = {.r = 8, .c = 6},
        .dir = {.dx = 0, .dy = 1},
        .hasGap = FALSE,
        .length = 4
    };
    test_assert(isFour(&board, &line3, PLAYER_X) == FALSE,
                "Should not detect four", results);
    test_assert(isFour(&board, &line4, PLAYER_X) == FALSE,
                "Should not detect four", results);
    test_assert(isFour(&board, &line5, PLAYER_X) == FALSE,
                "Should not detect four", results);
    
    test_end("IsFour");
}

void testIsMakingDoubleFour(TestResults* results) {
    test_begin("IsMakingDoubleFour");

    Board board;
    
    // テストケース1: 四四
    const char *testBoard1[] = {
        NULL,
        "..X......",
        "..@XXX...", 
        "..X......", 
        "..X......",
        ".........",
        "...X.....",
        "....X...",
        ".....X...",
        "...XXX@.."
    };
    
    initBoardWithStr(&board, testBoard1);
    test_assert(isMakingDoubleFour(&board, 2, 3, PLAYER_X) == TRUE,
                "Should detect double four", results);
    test_assert(isMakingDoubleFour(&board, 9, 7, PLAYER_X) == TRUE,
                "Should detect double four", results);
    
    // テストケース2: 四が1つだけ
    const char *testBoard2[] = {
        NULL,
        "..X......",
        ".O@XXXO..",
        "..X......",
        "..X......",
        ".........",
        ".....X...",
        "..XXX@.X.",
        ".....X...",
        ".....X..."
    };
    
    initBoardWithStr(&board, testBoard2);
    test_assert(isMakingDoubleFour(&board, 2, 3, PLAYER_X) == FALSE,
                "Should not detect double four", results);
    test_assert(isMakingDoubleFour(&board, 7, 6, PLAYER_X) == FALSE,
                "Should not detect double four", results);


    // テストケース3: 五を作っていても、この関数は気にせず四四を判定する
    const char *testBoard3[] = {
        NULL,
        ".XX......",
        ".X@X.X...",
        "..XX.....",
        "..X.X....",
        ".....X...",
        ".........",
        ".........",
        ".........",
        "........."
    };
    
    initBoardWithStr(&board, testBoard3);
    test_assert(isMakingDoubleFour(&board, 2, 3, PLAYER_X) == TRUE,
                "Should detect double four", results);

    // テストケース4: 一直線上の四四のケース
    const char *testBoard4[] = {
        NULL,
        "XXX.@.XXX",
        ".........",
        ".X.X@X.X.",
        ".........",
        ".XX.X@.XX",
        ".........",
        ".........",
        ".X.XX@.X.",
        "........."
    };
    
    initBoardWithStr(&board, testBoard4);
    test_assert(isMakingDoubleFour(&board, 1, 5, PLAYER_X) == TRUE,
                "Should detect double four", results);
    test_assert(isMakingDoubleFour(&board, 3, 5, PLAYER_X) == TRUE,
                "Should detect double four", results);
    test_assert(isMakingDoubleFour(&board, 5, 6, PLAYER_X) == TRUE,
                "Should detect double four", results);
    test_assert(isMakingDoubleFour(&board, 8, 6, PLAYER_X) == TRUE,
                "Should detect double four", results);

    // テストケース4: 白の手番
    test_assert(isMakingDoubleFour(&board, 1, 1, PLAYER_O) == FALSE,
                "Should not detect double four", results);
    test_end("IsMakingDoubleFour");
}

void testIsMakingGreatFour(TestResults* results) {
    test_begin("IsMakingGreatFour");

    Board board;
    
    // テストケース:
    // @は達四になるケース
    // #はならないケース
    // Ref (https://tokai-renjukai.pya.jp/siryo/RenjuKiso/RenjuKiso-1-2.pdf)
    const char *testBoard1[] = {
        NULL,
        ".#@XXX@#.",
        "....O....",
        ".O#XXX@#.",
        "...O.O...",
        ".........",
        ".........",
        ".........",
        "..OO@O.O.",
        "OOO@....."
    };
    
    initBoardWithStr(&board, testBoard1);
    test_assert(isMakingGreatFour(&board, 1, 2, PLAYER_X) == FALSE,
                "Should not detect great four", results);
    test_assert(isMakingGreatFour(&board, 1, 3, PLAYER_X) == TRUE,
                "Should detect great four", results);
    test_assert(isMakingGreatFour(&board, 1, 7, PLAYER_X) == TRUE,
                "Should detect great four", results);
    test_assert(isMakingGreatFour(&board, 1, 8, PLAYER_X) == FALSE,
                "Should not detect great four", results);
    test_assert(isMakingGreatFour(&board, 3, 3, PLAYER_X) == FALSE,
                "Should not detect great four", results);
    test_assert(isMakingGreatFour(&board, 3, 7, PLAYER_X) == TRUE,
                "Should detect great four", results);
    test_assert(isMakingGreatFour(&board, 3, 8, PLAYER_X) == FALSE,
                "Should not detect great four", results);
    test_assert(isMakingGreatFour(&board, 8, 5, PLAYER_O) == TRUE,
                "Should detect great four", results);
    test_assert(isMakingGreatFour(&board, 9, 4, PLAYER_O) == FALSE,
                "Should not detect great four", results);

    // テストケース2:
    const char *testBoard2[] = {
        NULL,
        ".........",
        "..OXXX##.",
        "....#....",
        ".#XX@X#..",
        "..#......",
        "#XX#.X.X.",
        ".........",
        ".........",
        "........."
    };
    
    initBoardWithStr(&board, testBoard2);
    test_assert(isMakingGreatFour(&board, 2, 7, PLAYER_X) == FALSE,
                "Should not detect great four", results);
    test_assert(isMakingGreatFour(&board, 2, 8, PLAYER_X) == FALSE,
                "Should not detect great four", results);
    test_assert(isMakingGreatFour(&board, 3, 5, PLAYER_X) == FALSE,
                "Should not detect great four", results);
    test_assert(isMakingGreatFour(&board, 4, 2, PLAYER_X) == FALSE,
                "Should not detect great four", results);
    test_assert(isMakingGreatFour(&board, 4, 5, PLAYER_X) == TRUE,
                "Should detect great four", results);
    test_assert(isMakingGreatFour(&board, 4, 7, PLAYER_X) == FALSE,
                "Should not detect great four", results);
    test_assert(isMakingGreatFour(&board, 5, 3, PLAYER_X) == FALSE,
                "Should not detect great four", results);
    test_assert(isMakingGreatFour(&board, 6, 1, PLAYER_X) == FALSE,
                "Should not detect great four", results);
    test_assert(isMakingGreatFour(&board, 6, 4, PLAYER_X) == FALSE,
                "Should not detect great four", results);
    test_end("IsMakingGreatFour");
}

void testIsThree(TestResults* results) {
    test_begin("IsThree");
    
    Board board;

    // テストケース:
    // Ref (https://tokai-renjukai.pya.jp/siryo/RenjuKiso/RenjuKiso-1-2.pdf)
    const char *testBoard1[] = {
        NULL,
        "..XXX....",
        ".....O.X.",
        ".........",
        "..XX.X...",
        "....X....",
        ".....O...",
        "........",
        ".........",
        ".O.OO....",
    };
    
    initBoardWithStr(&board, testBoard1);
    LineInfo line1 = {
        .start = {.r = 1, .c = 3},
        .end = {.r = 1, .c = 5},
        .dir = {.dx = 0, .dy = 1},
        .hasGap = FALSE,
        .length = 3
    };
    LineInfo line2 = {
        .start = {.r = 2, .c = 8},
        .end = {.r = 5, .c = 5},
        .dir = {.dx = 1, .dy = -1},
        .hasGap =TRUE, 
        .length = 3
    };
    LineInfo line3 = {
        .start = {.r = 4, .c = 3},
        .end = {.r = 4, .c = 6},
        .dir = {.dx = 0, .dy = 1},
        .hasGap = TRUE,
        .length = 3
    };
    LineInfo line4 = {
        .start = {.r = 9, .c = 2},
        .end = {.r = 9, .c = 5},
        .dir = {.dx = 0, .dy = 1},
        .hasGap = TRUE,
        .length = 3
    };

    test_assert(isThree(&board, &line1, PLAYER_X) == TRUE,
                "Should detect three", results);
    test_assert(isThree(&board, &line2, PLAYER_X) == TRUE,
                "Should detect three", results);
    test_assert(isThree(&board, &line3, PLAYER_X) == TRUE,
                "Should detect three", results);
    test_assert(isThree(&board, &line4, PLAYER_O) == TRUE,
                "Should detect three", results);

    const char *testBoard2[] = {
        NULL,
        ".XXX.O...",
        ".........",
        ".X..XXX.O",
        ".........",
        ".XX.X.X.O",
        ".........",
        ".........",
        ".........",
        ".........",
    };
    // 1行目は夏止め、
    // 3行目は長連筋の夏止め
    // 5行目は長連筋でそれぞれ三とはならない
    
    initBoardWithStr(&board, testBoard2);
    LineInfo line5 = {
        .start = {.r = 1, .c = 2},
        .end = {.r = 1, .c = 4},
        .dir = {.dx = 0, .dy = 1},
        .hasGap = FALSE,
        .length = 3
    };
    LineInfo line6 = {
        .start = {.r = 3, .c = 5},
        .end = {.r = 3, .c = 7},
        .dir = {.dx = 0, .dy = 1},
        .hasGap =FALSE, 
        .length = 3
    };
    LineInfo line7 = {
        .start = {.r = 5, .c = 2},
        .end = {.r = 5, .c = 5},
        .dir = {.dx = 0, .dy = 1},
        .hasGap = TRUE,
        .length = 3
    };

    test_assert(isThree(&board, &line5, PLAYER_X) == FALSE,
                "Should not detect three", results);
    test_assert(isThree(&board, &line6, PLAYER_X) == FALSE,
                "Should not detect three", results);
    test_assert(isThree(&board, &line7, PLAYER_X) == FALSE,
                "Should not detect three", results);

    const char *testBoard3[] = {
        NULL,
        ".........",
        ".XX.@....",
        "..OOB.X..",
        "...XXOX..",
        "...OXE.X.",
        "....#....",
        "...X.....",
        "....X....",
        ".........",
    };
    // Bにおいても、@が三三の禁手のため、達四にできない
    // Eにおいても、#が四四の禁手のため、達四にできない

    initBoardWithStr(&board, testBoard3);
    board.cells[3][5] = PLAYER_X;

    LineInfo line8 = {
        .start = {.r = 3, .c = 5},
        .end = {.r = 5, .c = 5},
        .dir = {.dx = 1, .dy = 0},
        .hasGap = FALSE,
        .length = 3};

    test_assert(isThree(&board, &line8, PLAYER_X) == FALSE,
                "Should not detect three", results);

    board.cells[3][5] = EMPTY_CELL;
    board.cells[5][6] = PLAYER_X;

    LineInfo line9 = {
        .start = {.r = 4, .c = 7},
        .end = {.r = 7, .c = 4},
        .dir = {.dx = 1, .dy = -1},
        .hasGap = TRUE,
        .length = 3};

    test_assert(isThree(&board, &line9, PLAYER_X) == FALSE,
                "Should not detect three", results);
    test_end("IsThree");
}

void testIsMakingDoubleThree(TestResults* results) {
    test_begin("IsMakingDoubleThree");

    Board board;
    
    // テストケース:
    // Ref (https://tokai-renjukai.pya.jp/siryo/RenjuKiso/RenjuKiso-1-2.pdf)
    const char *testBoard1[] = {
        NULL,
        ".........",
        "...X.....",
        "..X@X.O..",
        ".O.X.OO..",
        "....@X.X.",
        "...#.....",
        "....O....",
        ".........",
        ".........",
    };

    // @は黒の三三、#は白の三三
    
    initBoardWithStr(&board, testBoard1);
    test_assert(isMakingDoubleThree(&board, 3, 4, PLAYER_X) == TRUE,
                "Should detect double three", results);
    test_assert(isMakingDoubleThree(&board, 5, 5, PLAYER_X) == TRUE,
                "Should detect double three", results);
    test_assert(isMakingDoubleThree(&board, 6, 4, PLAYER_O) == TRUE,
                "Should detect double three", results);

    const char *testBoard2[] = {
        NULL,
        ".........",
        ".X.......",
        "X...O....",
        ".X.OO#...",
        "O..X.X@..",
        ".......O.",
        "..O...X..",
        "......X..",
        ".........",
    };

    // @は黒の三三、#は白の三三
    
    initBoardWithStr(&board, testBoard2);
    test_assert(isMakingDoubleThree(&board, 4, 6, PLAYER_O) == TRUE,
                "Should detect double three", results);
    test_assert(isMakingDoubleThree(&board, 5, 7, PLAYER_X) == TRUE,
                "Should detect double three", results);

    test_end("IsMakingDoubleThree");
}

void testHasLegalMove(TestResults* results) {
    test_begin("HasLegalMove");

    Board board = __prepareBoard();
    test_assert(hasLegalMove(&board, PLAYER_X) == TRUE,
                "Empty board should have legal moves for black", results);
    test_assert(hasLegalMove(&board, PLAYER_O) == TRUE,
                "Empty board should have legal moves for white", results);

    // 空点が (4,9) だけで、そこは黒にとって長連になる禁じ手。
    // 白には禁じ手が無いので白は打てる。
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

    test_assert(isProhibitedMove(&board, 4, BOARD_COLUMNS, PLAYER_X) == TRUE,
                "Test setup: the only empty cell should be prohibited for black", results);
    test_assert(hasLegalMove(&board, PLAYER_X) == FALSE,
                "Black should have no legal move when every empty cell is prohibited", results);
    test_assert(hasLegalMove(&board, PLAYER_O) == TRUE,
                "White has no forbidden moves, so an empty cell is always legal", results);

    // 満局
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
    initBoardWithStr(&board, full);

    test_assert(hasLegalMove(&board, PLAYER_X) == FALSE,
                "Full board should have no legal move for black", results);
    test_assert(hasLegalMove(&board, PLAYER_O) == FALSE,
                "Full board should have no legal move for white", results);

    test_end("HasLegalMove");
}

int runBoardTests(void) {
    TestResults results = {0, 0, 0};
    test_suite_begin("Board Tests");
    
    suppress_output();
    
    testInitBoard(&results);
    testIsWinMove(&results);
    testIsMakingOverLine(&results);
    testCountContinuousStonesWithGap(&results);
    testIsOpenLine(&results);
    testIsHalfOpenLine(&results);
    testIsHalfOpenLineAtBoardEdge(&results);
    testGetGapIdx(&results);
    testIsFour(&results);
    testIsMakingDoubleFour(&results);
    testIsMakingGreatFour(&results);
    testIsThree(&results);
    testIsMakingDoubleThree(&results);
    testHasLegalMove(&results);
    
    restore_output();
    
    test_suite_end("Board Tests", &results);
    return results.failed;
}
