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

/* TODO */
// TODO: handle queue function errors, return -1 if needed
#define NUM_QUEUES 3

enum state{READY, BLOCKED, ZOMBIE, RUNNING};

typedef struct tcb {
	uthread_t tid;
	int state;
	uthread_ctx_t ctx;
	void *stack;
} tcb;

typedef tcb* tcb_t;

uthread_t num_thr = 0; // number of threads created 
queue_t scheduler[NUM_QUEUES];
tcb_t main_thr; // main thread
tcb_t curr_thr; // currently active and running thread

int uthread_start(int preempt)
{
	(void)preempt;
	/* TODO */
	for (int i = 0; i < NUM_QUEUES; i++) {
		scheduler[i] = queue_create();
	}
	main_thr = malloc(sizeof(tcb_t));
	if (main_thr == NULL) return -1;
	main_thr->tid = num_thr;
	main_thr->state = RUNNING;
	main_thr->stack = uthread_ctx_alloc_stack();
	curr_thr = main_thr;
	return 0;
}

int uthread_stop(void)
{
	/* TODO: when to return -1? */
	if (curr_thr->tid != 0) return -1;
	if (queue_length(scheduler[READY]) > 0 || queue_length(scheduler[ZOMBIE]) > 0) return -1;
	for (int i = 0; i < NUM_QUEUES; i++) {
		queue_destroy(scheduler[i]);
	}
	uthread_ctx_destroy_stack(curr_thr->stack);
	free(curr_thr);
	return 0;
}

int uthread_create(uthread_func_t func)
{
	/* TODO */
	if (num_thr == USHRT_MAX) return -1;
	tcb_t thr = malloc(sizeof(tcb_t));
	if (thr == NULL) return -1;
	thr->tid = ++num_thr;
	thr->state = READY;
	thr->stack = uthread_ctx_alloc_stack();
	if (uthread_ctx_init(&thr->ctx, thr->stack, func)) return -1;
	if (queue_enqueue(scheduler[READY], thr)) return -1;
	return thr->tid;
}

void uthread_yield(void)
{
	/* TODO */
	tcb_t prev_thr = curr_thr;                                             
	if (queue_dequeue(scheduler[READY], (void**)&curr_thr) == -1) { // if no more threads ready, back to main thread
		curr_thr = main_thr;
	}
	if (prev_thr->state != ZOMBIE) { // round-robin put back into queue if previous thread not a zombie
		prev_thr->state = READY;
		queue_enqueue(scheduler[READY], prev_thr);
	}
	curr_thr->state = RUNNING;
	// printf("context switch: T%d to T%d\n", prev_thr->tid, curr_thr->tid);
	uthread_ctx_switch(&prev_thr->ctx, &curr_thr->ctx);
	// printf("after context switch\n");
}

uthread_t uthread_self(void)
{
	/* TODO */
	return curr_thr->tid;
}

void uthread_exit(int retval)
{
	(void)retval;
	/* TODO */
	curr_thr->state = ZOMBIE;
	queue_enqueue(scheduler[ZOMBIE], curr_thr);
	uthread_yield();
	// printf("this should not print\n");
}

int uthread_join(uthread_t tid, int *retval)
{
	(void)tid;
	(void)retval;
	/* TODO */
	while (1) {
		if (queue_length(scheduler[READY]) == 0) {
			break;
		}
		uthread_yield();
	}
	return -1;
}

