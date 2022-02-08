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
	int retval;
	uthread_t joined_tid; // tid of calling thread that joined it
} tcb;

typedef tcb* tcb_t;

uthread_t num_thr = 0; // number of threads created 
queue_t scheduler[NUM_QUEUES];
tcb_t main_thr; // main thread
tcb_t curr_thr; // currently active and running thread

/** 
 * Finds thread that has TID @tid_to_find 
 * @return 0 if no match or thread is already joined; 1 otherwise
 **/
int find_thread(queue_t q, void *data, void *tid_to_find)
{
	uthread_t tid = (uthread_t)tid_to_find;
	tcb_t thr = (tcb_t)data;
	(void)q; // unused

	if (thr->tid == tid) { // tid match found
		return 1;
	}
	return 0;
}

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
	thr->retval = NULL;
	thr->joined_tid = -1;
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
	// Round-robin put back into queue if previous thread not a zombie or blocked
	if (prev_thr->state != ZOMBIE && prev_thr->state != BLOCKED) {
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
	curr_thr->retval = retval;
	queue_enqueue(scheduler[ZOMBIE], curr_thr);
	
	// Find calling thread in blocked queue and move to ready queue (if applicable)
	tcb_t calling_thr = NULL;
	if (curr_thr->joined_tid != -1) {
		queue_iterate(scheduler[BLOCKED], find_thread, curr_thr->joined_tid, (void **)&calling_thr);
		if (calling_thr) {
			queue_delete(scheduler[BLOCKED], calling_thr);
			calling_thr->state = READY;
			queue_enqueue(scheduler[READY], calling_thr);
		}
	}

	uthread_yield();
	// printf("this should not print\n");
}

int uthread_join(uthread_t tid, int *retval)
{
	/* TODO */
	if (tid == 0) return -1;
	if (tid == curr_thr->tid) return -1; // if tid is tid of calling thread, return -1

	tcb_t ptr = NULL;

	// Search for active thread tid
	queue_iterate(scheduler[READY], find_thread, (void *)tid, (void **)&ptr);
	if (ptr) {
		if (ptr->joined_tid == -1) { // if thread tid not already joined
			ptr->joined_tid = curr_thr->tid;
			curr_thr->state = BLOCKED; // block calling thread
			queue_enqueue(scheduler[BLOCKED], curr_thr);
			uthread_yield();
		} else { // if thread tid already joined
			return -1;
		}
	}

	// Search for thread tid in zombie queue and collect retval if found
	queue_iterate(scheduler[ZOMBIE], find_thread, (void *)tid, (void **)&ptr);
	if (ptr) { // if return value collected from a zombie thread
		queue_delete(scheduler[ZOMBIE], ptr);
		*retval = (int)ptr->retval;
		uthread_ctx_destroy_stack(ptr->stack);
		free(ptr);
		ptr = NULL;
		return 0;
	}

	return -1;
}

