#include <stdio.h>
#include "../include/queue.h"
#include "test_utils.h"
#include "test_queue.h"

void testQueueInitialization(TestResults* results) {
    test_begin("QueueInitialization");
    
    Queue q;
    initQueue(&q);

    test_assert(q.size == 0, 
                "Queue size should be 0 after initialization", results);
    test_assert(q.head == 0, 
                "Queue head should be 0 after initialization", results);
    test_assert(q.tail == 0, 
                "Queue tail should be 0 after initialization", results);
    test_assert(q.capacity == QUEUE_SIZE, 
                "Queue capacity should be QUEUE_SIZE after initialization", results);

    freeQueue(&q);
    
    test_end("QueueInitialization");
}

void testQueuePushPop(TestResults* results) {
    test_begin("QueuePushPop");
    
    Queue q;
    initQueue(&q);

    Position p1 = {1, 2};
    Position p2 = {3, 4};
    Position p3 = {5, 6};

    push(&q, p1);
    push(&q, p2);
    push(&q, p3);

    test_assert(q.size == 3, 
                "Queue size should be 3 after pushing 3 elements", results);

    Position out1 = pop(&q);
    test_assert(out1.x == 1 && out1.y == 2, 
                "First popped element should match first pushed element", results);
    test_assert(q.size == 2, 
                "Queue size should be 2 after one pop", results);

    Position out2 = pop(&q);
    test_assert(out2.x == 3 && out2.y == 4, 
                "Second popped element should match second pushed element", results);
    test_assert(q.size == 1, 
                "Queue size should be 1 after two pops", results);

    Position out3 = pop(&q);
    test_assert(out3.x == 5 && out3.y == 6, 
                "Third popped element should match third pushed element", results);
    test_assert(q.size == 0, 
                "Queue size should be 0 after three pops", results);
    test_assert(isEmpty(&q) == TRUE, 
                "Queue should be empty after popping all elements", results);

    freeQueue(&q);
    
    test_end("QueuePushPop");
}

void testQueueExtend(TestResults* results) {
    test_begin("QueueExtend");

    Queue q;
    initQueue(&q);

    // Fill queue to initial capacity
    for (int i = 0; i < QUEUE_SIZE; i++) {
        push(&q, (Position){i, i * 2});
    }

    test_assert(q.size == QUEUE_SIZE, 
                "Queue size should equal QUEUE_SIZE after filling", results);
    test_assert(q.capacity == QUEUE_SIZE, 
                "Queue capacity should still be QUEUE_SIZE", results);
    test_assert(isFull(&q) == TRUE, 
                "Queue should be full", results);

    // Test extension
    push(&q, (Position){100, 200});

    test_assert(q.size == QUEUE_SIZE + 1, 
                "Queue size should increase after extension", results);
    test_assert(q.capacity == QUEUE_SIZE * 2, 
                "Queue capacity should double after extension", results);
    test_assert(isFull(&q) == FALSE, 
                "Queue should not be full after extension", results);

    // Verify all elements
    for (int i = 0; i <= QUEUE_SIZE; i++) {
        Position p = pop(&q);
        test_assert(p.x == i && p.y == i * 2, 
                    "Queue elements should maintain order after extension", results);
    }

    test_assert(isEmpty(&q) == TRUE, 
                "Queue should be empty after popping all elements", results);

    freeQueue(&q);
    
    test_end("QueueExtend");
}

int runQueueTests(void) {
    TestResults results = {0, 0, 0};
    test_suite_begin("Queue Tests");
    
    suppress_output();
    
    testQueueInitialization(&results);
    testQueuePushPop(&results);
    testQueueExtend(&results);
    
    restore_output();
    
    test_suite_end("Queue Tests", &results);
    return results.failed;
}
