#include <assert.h>
#include <limits.h>
#include <signal.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>

#include "private.h"
#include "queue.h"
#include "uthread.h"

#define ALL 0
#define READY 1
#define ZOMBIE 2
#define EXITED 3
#define RUN 4
#define NUM_QUEUE 4

/* TODO */
typedef struct TCB {
	uthread_t tid;
	void *stack;
	uthread_ctx_t ctx;
	int state;
} TCB;

queue_t q[NUM_QUEUE];
TCB main_thread;
TCB running_thread;

static int destroy_stack_all(queue_t q, void *data, void *arg)
{
    TCB *tcb = (TCB*)data;
		uthread_ctx_destroy_stack(tcb->stack);

    return 0;
}

static int dequeue_all(queue_t q, void *data, void *arg)
{
    int *ptr;
		if (!(queue_dequeue(q, (void**)&ptr))) return -1;

    return 0;
}

/*
 * uthread_start - Start the multithreading library
 * @preempt: Preemption enable
 *
 * This function should only be called by the process' original execution
 * thread. It starts the multithreading scheduling library, and registers the
 * calling thread as the 'main' user-level thread (TID 0). If @preempt is
 * `true`, then preemptive scheduling is enabled.
 *
 * Return: 0 in case of success, -1 in case of failure (e.g., memory
 * allocation).
 */
int uthread_start(int preempt)
{
	/* TODO */
	for (int i = 0; i < NUM_QUEUE; i++) {
		q[i] = queue_create();
	}

	TCB main;
	main.tid = 0;
	if (!(main.stack = uthread_ctx_alloc_stack())) return -1;
	main.state = RUN;

	running_thread.state = READY;
	
	if (!(queue_enqueue(q[ALL], &main))) return -1;
	main_thread = main;

	return 0;
}

/*
 * uthread_stop - Stop the multithreading library
 *
 * This function should only be called by the main execution thread of the
 * process. It stops the multithreading scheduling library if there are no more
 * user threads.
 *
 * Return: 0 in case of success, -1 in case of failure.
 */
int uthread_stop(void)
{
	/* TODO */
	if (running_thread.tid != 0 || queue_length(q[READY]) > 1) return -1;

	queue_iterate(q[ALL], destroy_stack_all, NULL, NULL);
	for (int i = 0; i < NUM_QUEUE; i++) {
		if (!(queue_iterate(q[i], dequeue_all, NULL, NULL))) return -1;
	}
	for (int i = 0; i < NUM_QUEUE; i++) {
		if (!(queue_destroy(q[i]))) return -1;
	}
	
	return 0;
}

/*
 * uthread_create - Create a new thread
 * @func: Function to be executed by the thread
 *
 * This function creates a new thread running the function @func and returns the
 * TID of this new thread.
 *
 * Return: -1 in case of failure (memory allocation, context creation, TID
 * overflow, etc.), or the TID of the new thread.
 */
int uthread_create(uthread_func_t func)
{
	/* TODO */
	//
	TCB new_thread;
	new_thread.tid = queue_length(q[ALL]);
	if (!(new_thread.stack = uthread_ctx_alloc_stack())) return -1;
	if (uthread_ctx_init(&new_thread.ctx, new_thread.stack, func) == -1) return -1;

	if (running_thread.state == READY) {
		new_thread.state = RUN;
		running_thread = new_thread;
	} else {
		new_thread.state = READY;
		if (!(queue_enqueue(q[READY], &new_thread))) return -1;
	}
	if (!(queue_enqueue(q[ALL], &new_thread))) return -1;
	return new_thread.tid;
}

/*
 * uthread_yield - Yield execution
 *
 * This function is to be called from the currently active and running thread in
 * order to yield for other threads to execute.
 */
void uthread_yield(void)
{
	/* TODO */
	
}

/*
 * uthread_self - Get thread identifier
 *
 * Return: The TID of the currently running thread
 */
uthread_t uthread_self(void)
{
	/* TODO */
	return running_thread.tid;
}

/*
 * uthread_exit - Exit from currently running thread
 * @retval: Return value
 *
 * This function is to be called from the currently active and running thread in
 * order to finish its execution. The return value @retval is to be collected
 * from a joining thread.
 *
 * A thread which has not been 'collected' should stay in a zombie state. This
 * means that until collection, the resources associated to a zombie thread
 * should not be freed.
 *
 * This function shall never return.
 */
void uthread_exit(int retval)
{
	/* TODO */
}

/*
 * uthread_join - Join a thread
 * @tid: TID of the thread to join
 * @retval: Address of an integer that will receive the return value
 *
 * This function makes the calling thread wait for the thread @tid to complete
 * and assign the return value of the finished thread to @retval (if @retval is
 * not NULL).
 *
 * A thread can be joined by only one other thread.
 *
 * Return: -1 if @tid is 0 (the 'main' thread cannot be joined), if @tid is the
 * TID of the calling thread, if thread @tid cannot be found, or if thread @tid
 * is already being joined. 0 otherwise.
 */
int uthread_join(uthread_t tid, int *retval)
{
	/* TODO */
	while (1) {

	}
	return -1;
}

