#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include "private.h"
#include "queue.h"
#include "uthread.h"

#define TEST_ASSERT(assert)						\
do {																	\
	printf("ASSERT: " #assert " ... ");	\
	if (assert) {												\
		printf("PASS\n");									\
	} else	{														\
		printf("FAIL\n");									\
		exit(1);													\
	}																		\
} while(0)


int thread2(void)
{
	printf("%s:%d: thread2\n", __FILE__, __LINE__);
	return 2;
}

/* Thread that never yields (has infinite loop) */
int thread1(void)
{
	while (1);
	return 1;
}

/* Test to see that main thread and thread2 make progress even though thread1 is in an infinite loop */
void test_infinite_loop(void)
{
	fprintf(stderr, "*** TEST infinite_loop ***\n");

	uthread_t tid;
	int retval;

	uthread_start(1); // enable preemption
	uthread_create(thread1); // thread1 tid not used
	tid = uthread_create(thread2);
	uthread_yield(); // main thread yield to thread1
	TEST_ASSERT(uthread_join(tid, &retval) == 0);
	TEST_ASSERT(retval == 2);
	TEST_ASSERT(uthread_stop() == -1); // thread1 never ends, still in ready queue
}

int main(void)
{
	test_infinite_loop();
	return 0;
}