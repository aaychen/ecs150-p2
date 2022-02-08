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

int ret_not_zero(void)
{
	printf("%s:%d: Returning -5\n", __FILE__, __LINE__);
    return -5;
}

void test_single_thr(void)
{
    fprintf(stderr, "*** TEST single_thr ***\n");

	uthread_t tid;
	int retval;

	uthread_start(0);
	tid = uthread_create(hello);
	uthread_join(tid, &retval);
    TEST_ASSERT(retval == 0);
    uthread_stop();
}

void test_single_thr_nonzero_retval(void)
{
    fprintf(stderr, "*** TEST single_thr_nonzero_retval ***\n");

	uthread_t tid;
	int retval;

	uthread_start(0);
	tid = uthread_create(ret_not_zero);
	uthread_join(tid, &retval);
    TEST_ASSERT(retval == -5);
    uthread_stop();
}

int thread3(void)
{
	uthread_yield();
    printf("%s:%d: thread%d\n", __FILE__, __LINE__, uthread_self());
	return 0;
}

int thread2(void)
{
	uthread_create(thread3);
	uthread_yield();
	printf("%s:%d: thread%d\n", __FILE__, __LINE__, uthread_self());
	return 0;
}

int thread1(void)
{
	uthread_create(thread2);
	uthread_yield();
	printf("%s:%d: thread%d\n", __FILE__, __LINE__, uthread_self());
	uthread_yield();
	return 10;
}

void test_multiple_thr(void)
{
    fprintf(stderr, "*** TEST multiple_thr ***\n");
	int retval;

	uthread_start(0);
	uthread_join(uthread_create(thread1), &retval);
    TEST_ASSERT(retval == 10);
    uthread_stop();
}

int main(void)
{
    test_single_thr();
    test_single_thr_nonzero_retval();
    test_multiple_thr();
	return 0;
}
