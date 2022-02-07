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
	int retval;
} TCB;

typedef struct TCB* TCB_t;

TCB_t thread_create(int tid, uthread_func_t func) {
	TCB_t new_thread = (TCB_t)malloc(sizeof(TCB));
	if (!new_thread) return NULL;
	new_thread->tid = tid;
	if (!(new_thread->stack = uthread_ctx_alloc_stack())) return NULL;
	if (uthread_ctx_init(&new_thread->ctx, new_thread->stack, func) == -1) return NULL;
	new_thread->state= READY;

	return new_thread;
}

queue_t q[NUM_QUEUE];
TCB_t main_thread;
TCB_t running_thread;
TCB_t temp;

static int destroy_stack_all(queue_t q, void *data, void *arg)
{
		(void)q; //unused
		(void)arg; //unused
    TCB_t* temp = (TCB_t*)data;
		TCB_t thread = *temp;
		uthread_ctx_destroy_stack(thread->stack);

    return 0;
}

static int dequeue_all(queue_t q, void *data, void *arg)
{
		(void)data; //unused
		(void)arg; //unused
    int *ptr;
		return queue_dequeue(q, (void**)&ptr);

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
	(void)preempt; //unused

	for (int i = 0; i < NUM_QUEUE; i++) {
		q[i] = queue_create();
	}

	main_thread = (TCB_t)malloc(sizeof(TCB));
	if (!main_thread) return -1;
	main_thread->tid = 0;
	if (!(main_thread->stack = uthread_ctx_alloc_stack())) return -1;
	main_thread->state = RUN;
	running_thread = main_thread;
	
	return queue_enqueue(q[ALL], &main_thread);
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
	if (running_thread->tid != 0 || queue_length(q[READY]) > 1 || queue_length(q[ZOMBIE]) > 0) return -1;

	if (queue_iterate(q[ALL], destroy_stack_all, NULL, NULL) == -1) return -1;
	for (int i = 0; i < NUM_QUEUE; i++) {
		if (queue_iterate(q[i], dequeue_all, NULL, NULL) == -1) return -1;
	}
	for (int i = 0; i < NUM_QUEUE; i++) {
		if (queue_destroy(q[i]) == -1 ) return -1;
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
	temp = thread_create(queue_length(q[ALL]), func);
	if (queue_enqueue(q[READY], &temp) == -1) return -1;
	if (queue_enqueue(q[ALL], &temp) == -1) return -1;

	return temp->tid;
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
	TCB_t prev_running = running_thread, *ptr;

	if (queue_dequeue(q[READY], (void**)&ptr) == -1)
		uthread_ctx_switch(&prev_running->ctx, &main_thread->ctx);
	running_thread = *ptr;
	running_thread->state = RUN;
	if (prev_running != main_thread && prev_running->state != ZOMBIE) {
		prev_running->state = READY;
		if (queue_enqueue(q[READY], &prev_running) == -1) exit(1);
	}
	uthread_ctx_switch(&prev_running->ctx, &running_thread->ctx);
}

/*
 * uthread_self - Get thread identifier
 *
 * Return: The TID of the currently running thread
 */
uthread_t uthread_self(void)
{
	/* TODO */
	return running_thread->tid;
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
	running_thread->state = ZOMBIE;
	running_thread->retval = retval;
	if (queue_enqueue(q[ZOMBIE], &running_thread) == -1) exit (1);
	uthread_yield();
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
	(void)tid; //unused
	(void)retval; //unused

	while (1) {
		if (queue_length(q[READY]) < 1) break;
		uthread_yield();
	}
	
	return 0;
}

