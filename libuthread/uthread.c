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

#define NUM_QUEUES 3

enum state{READY, BLOCKED, ZOMBIE, RUNNING};

typedef struct tcb {
	uthread_t tid;
	int state;
	uthread_ctx_t ctx;
	void *stack;
	int retval;
	uthread_t joining_thr_tid; // tid of calling thread that joined it
} tcb;

typedef tcb* tcb_t;

uthread_t num_thr = 0; // number of threads created 
queue_t scheduler[NUM_QUEUES];
tcb_t main_thr; // main thread
tcb_t curr_thr; // currently active and running thread
int scheduler_preempt;

/** 
 * Finds thread that has TID @tid_to_find 
 * @return 0 if no match; 1 if match found
 **/
int find_thread(queue_t q, void *data, void *tid_to_find)
{
	if (data == NULL || tid_to_find == NULL) return 0;

	uthread_t tid = *(uthread_t*)tid_to_find;
	tcb_t thr = (tcb_t)data;
	(void)q; // unused

	if (thr->tid == tid) { // tid match found
		return 1;
	}
	return 0;
}

int uthread_start(int preempt)
{
	// Create queues
	for (int i = 0; i < NUM_QUEUES; i++) {
		scheduler[i] = queue_create();
		if (scheduler[i] == NULL) {
			return -1;
		}
	}

	// "Initialize" main thread
	main_thr = malloc(sizeof(tcb));
	if (main_thr == NULL) return -1;
	main_thr->tid = num_thr;
	main_thr->state = RUNNING;
	main_thr->stack = uthread_ctx_alloc_stack();
	if (main_thr->stack == NULL) return -1;
	
	// Set current active thread to main thread
	curr_thr = main_thr;

	// Check if preemption was enabled
	if ((scheduler_preempt = preempt)) preempt_start();

	return 0;
}

int uthread_stop(void)
{
	// Disable preemption if needed
	if (scheduler_preempt == 1) preempt_stop();

	if (curr_thr->tid != main_thr->tid) return -1;

	// Check if there are still threads left
	if (queue_length(scheduler[READY]) > 0 || queue_length(scheduler[ZOMBIE]) > 0 || queue_length(scheduler[BLOCKED]) > 0) {
		return -1;
	}

	for (int i = 0; i < NUM_QUEUES; i++) {
		queue_destroy(scheduler[i]);
	}
	uthread_ctx_destroy_stack(curr_thr->stack);
	free(curr_thr); // main_thr and curr_thr should point to same thing at this point (main thread's tcb struct)
	num_thr = 0; // reset when stopping uthread library

	return 0;
}

int uthread_create(uthread_func_t func)
{
	preempt_disable();
	if (num_thr == USHRT_MAX) return -1;

	tcb_t thr = malloc(sizeof(tcb));
	if (thr == NULL) return -1;
	thr->tid = ++num_thr;
	preempt_enable();
	thr->state = READY;
	thr->stack = uthread_ctx_alloc_stack();
	if (thr->stack == NULL) return -1;
	thr->joining_thr_tid = thr->tid;
	if (uthread_ctx_init(&thr->ctx, thr->stack, func) == -1) return -1;
	if (queue_enqueue(scheduler[READY], thr) == -1) return -1;
	return thr->tid;
}

void uthread_yield(void)
{
	preempt_disable(); // already yielding so don't force to yield again

	tcb_t prev_thr = curr_thr;

	// Round-robin put back into ready queue if previous thread is not a zombie or blocked
	// If previous thread is a zombie or blocked, already enqueued into the appropriate queue (in exit and join functions)
	if (prev_thr->state != ZOMBIE && prev_thr->state != BLOCKED) {
		prev_thr->state = READY;
		queue_enqueue(scheduler[READY], prev_thr);
	}

	if (queue_dequeue(scheduler[READY], (void**)&curr_thr) == -1) { // if no more threads in ready queue, do nothing and continue
		return;
	}
	curr_thr->state = RUNNING;
	uthread_ctx_switch(&prev_thr->ctx, &curr_thr->ctx);
	preempt_enable();
}

uthread_t uthread_self(void)
{
	return curr_thr->tid;
}

void uthread_exit(int retval)
{
	preempt_disable();
	curr_thr->state = ZOMBIE;
	curr_thr->retval = retval;
	queue_enqueue(scheduler[ZOMBIE], curr_thr);
	
	// Find joining thread in blocked queue and move to ready queue (if applicable)
	tcb_t joining_thr = NULL;

	if (curr_thr->joining_thr_tid != uthread_self()) { // if has calling thread to collect its return value
		queue_iterate(scheduler[BLOCKED], find_thread, &curr_thr->joining_thr_tid, (void **)&joining_thr);
		if (joining_thr) { // unblock joining thread and enqueue into ready queue
			queue_delete(scheduler[BLOCKED], joining_thr);
			joining_thr->state = READY;
			queue_enqueue(scheduler[READY], joining_thr);
		}
	}
	preempt_enable();
	uthread_yield();
}

int uthread_join(uthread_t tid, int *retval)
{
	if (tid == 0 || tid == uthread_self()) return -1; // main thread and self thread check

	tcb_t target = NULL;

	// Search for target thread in active queues
	queue_iterate(scheduler[READY], find_thread, &tid, (void **)&target);
	if (target == NULL) queue_iterate(scheduler[BLOCKED], find_thread, &tid, (void **)&target); // search blocked queue if target thread not in ready queue
	
	if (target) {
		if (target->joining_thr_tid == target->tid) { // if thread tid not already joined
			target->joining_thr_tid = uthread_self();
			curr_thr->state = BLOCKED; // block calling thread
			queue_enqueue(scheduler[BLOCKED], curr_thr);
			uthread_yield();
		} else { // if thread tid already being joined
			return -1;
		}
	}

	// Search for thread tid in zombie queue and collect retval if found
	// This block also runs when calling thread is unblocked. When calling thread unblocked, target thread should be a zombie.
	queue_iterate(scheduler[ZOMBIE], find_thread, &tid, (void **)&target);
	if (target && (target->joining_thr_tid == target->tid || target->joining_thr_tid == uthread_self())) {
		queue_delete(scheduler[ZOMBIE], target);
		if (retval != NULL) *retval = target->retval;
		uthread_ctx_destroy_stack(target->stack);
		free(target);
		target = NULL;
		return 0;
	}

	return -1;
}

