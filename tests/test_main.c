#include <stdio.h>
#include "test_utils.h"
#include "test_runner.h"
#include "test_queue.h"
#include "test_board.h"
#include "test_ui.h"
#include "test_game.h"
#include "test_cpu.h"

int main() {
    int failed = 0;

    fprintf(stderr, "Starting All Tests...\n\n");
    failed += runRunnerTests();
    failed += runUtilsTests();
    failed += runQueueTests();
    failed += runBoardTests();
    failed += runUiTests();
    failed += runGameTests();
    failed += runCPUTests();

    fprintf(stderr, "\nAll Tests Completed. (%d failed)\n", failed);
    return exitCodeFor(failed);
}
