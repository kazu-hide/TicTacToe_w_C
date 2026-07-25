#include <stdio.h>
#include "test_utils.h"
#include "test_runner.h"

/*
 * テストランナー自身の検証。
 * 失敗を1件でも記録したら make test が非 0 で終わること (= CI で気づけること) を保証する。
 */

void testSuiteExitCodeIsZeroWhenAllPass(TestResults* results) {
    test_begin("SuiteExitCodeIsZeroWhenAllPass");

    TestResults sample = {0, 0, 0};
    test_assert(TRUE, "passing assertion", &sample);
    test_assert(TRUE, "passing assertion", &sample);

    test_assert(sample.failed == 0,
                "All passing assertions should record no failure", results);
    test_assert(exitCodeFor(sample.failed) == 0,
                "Exit code should be 0 when nothing failed", results);

    test_end("SuiteExitCodeIsZeroWhenAllPass");
}

void testSuiteExitCodeIsNonZeroWhenAnyFails(TestResults* results) {
    test_begin("SuiteExitCodeIsNonZeroWhenAnyFails");

    // 失敗を意図的に記録する。ランナーの検証が目的なので、
    // このスイート自身の集計には含めない
    TestResults sample = {0, 0, 0};
    suppress_stderr();
    test_assert(FALSE, "intentional failure for runner verification", &sample);
    restore_stderr();

    test_assert(sample.failed == 1,
                "A failing assertion should be recorded", results);
    test_assert(exitCodeFor(sample.failed) != 0,
                "Exit code should be non-zero when something failed", results);

    test_end("SuiteExitCodeIsNonZeroWhenAnyFails");
}

int runRunnerTests(void) {
    TestResults results = {0, 0, 0};
    test_suite_begin("Runner Tests");

    suppress_output();

    testSuiteExitCodeIsZeroWhenAllPass(&results);
    testSuiteExitCodeIsNonZeroWhenAnyFails(&results);

    restore_output();

    test_suite_end("Runner Tests", &results);
    return results.failed;
}
