/**
 * Tests creations of threads and successful returns.
 */
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include <uthread.h>

#define TEST_ASSERT(assert)				\
do {									\
	printf("ASSERT: " #assert " ... ");	\
	if (assert) {						\
		printf("PASS\n");				\
	} else	{							\
		printf("FAIL\n");				\
		exit(1);						\
	}									\
} while(0)

int hello(void)
{
	printf("%s:%d: Hello world!\n", __FILE__, __LINE__);
	return 0;
}

int five(void)
{
	printf("%s:%d: Returning 5\n", __FILE__, __LINE__);
    return 5;
}

/* Test creation of a single thread and collection of return value */
void test_single_thr(void)
{
    fprintf(stderr, "*** TEST single_thr ***\n");

	uthread_t tid;
	int retval;

	uthread_start(0);
	tid = uthread_create(hello);
	TEST_ASSERT(uthread_join(tid, &retval) == 0);
    TEST_ASSERT(retval == 0);

    tid = uthread_create(five);
    TEST_ASSERT(uthread_join(tid, &retval) == 0);
    TEST_ASSERT(retval == 5);

    TEST_ASSERT(uthread_stop() == 0);
}

int mjr_thr2(void)
{
    int retval = -10;
    TEST_ASSERT(uthread_join(1, &retval) == -1);
    TEST_ASSERT(retval == -10);
    uthread_yield();
    uthread_yield();
    return 2;
}

int mjr_thr1(void)
{
    uthread_yield();
    return 1;
}

/**
 * Tests two threads joining one (not main) thread
 * In the end, main thread will have collected the two threads.
 */
void test_multiple_joining_thr(void)
{
    fprintf(stderr, "*** TEST multiple_joining_thr ***\n");

	uthread_t tid1, tid2;
	int retval1, retval2;

	uthread_start(0);
    tid1 = uthread_create(mjr_thr1);
    tid2 = uthread_create(mjr_thr2);
	uthread_join(tid1, &retval1);
    TEST_ASSERT(retval1 == 1);
    uthread_join(tid2, &retval2);
    TEST_ASSERT(retval2 == 2);
    TEST_ASSERT(uthread_stop() == 0);
}

/**
 * Tests collecting from an already dead (zombie) thread
 */
void test_collect_dead_thr(void) {
    fprintf(stderr, "*** TEST collect_dead_thr ***\n");

	uthread_t tid;
	int retval;

	uthread_start(0);
	tid = uthread_create(hello);
    uthread_yield();
	TEST_ASSERT(uthread_join(tid, &retval) == 0);
    TEST_ASSERT(retval == 0);
    TEST_ASSERT(uthread_stop() == 0);
}

int thread3(void)
{
	uthread_yield();
    printf("%s:%d: thread%d\n", __FILE__, __LINE__, uthread_self());
	return 3;
}

int thread2(void)
{
	uthread_create(thread3);
	uthread_yield();
	printf("%s:%d: thread%d\n", __FILE__, __LINE__, uthread_self());
	return 2;
}

int thread1(void)
{
    int retval;
    uthread_join(uthread_create(thread2), &retval);;
	uthread_yield(); // thread1 yielded to thread2
	printf("%s:%d: thread%d\n", __FILE__, __LINE__, uthread_self());
	uthread_yield();
	return 1;
}

/* Test creating multiple threads and collection of return value */
void test_multiple_thr(void)
{
    fprintf(stderr, "*** TEST multiple_thr ***\n");
	int retval;

	uthread_start(0);
	uthread_join(uthread_create(thread1), &retval); // main yielded to thread1
    TEST_ASSERT(retval == 1);
    uthread_stop();
}

int main(void)
{
    test_single_thr();
    test_multiple_joining_thr();
    test_collect_dead_thr();
    // test_multiple_thr();
	return 0;
}
