#include <signal.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>

#include "private.h"
#include "uthread.h"

/*
 * Frequency of preemption
 * 100Hz is 100 times per second
 */
#define HZ 100
#define INTERVAL 10		/* number of milliseconds to go off */

void timer_handler(int signum){
	printf("%s:%d: Inside timer handler function\n", __FILE__, __LINE__);
	uthread_yield();
}

struct sigaction new_act;
struct sigaction old_act; // to store previous signal action
struct itimerval new_timer;
struct itimerval old_timer; // to store previous timer configuration
sigset_t block_timer;

void preempt_start(void)
{
	/* TODO */
	// Set up sigaction
	new_act.sa_handler = timer_handler; // set the handler
  sigemptyset(&new_act.sa_mask); // no signal is blocked while the handler runs
  new_act.sa_flags = 0; // no flag
	sigaction(SIGVTALRM, &new_act, &old_act);

	// Configure timer
	// First timer interrupt after 10 msec
	new_timer.it_value.tv_sec = 0;
	new_timer.it_value.tv_usec = INTERVAL * 1000;
	// Successive timer interrupts every 10 msec after that
	new_timer.it_interval = new_timer.it_value;
	
	setitimer(ITIMER_VIRTUAL, &new_timer, &old_timer);
}

void preempt_stop(void)
{
	/* TODO */
	// Restore previous signal action
	sigaction(SIGVTALRM, &old_act, NULL);
	// Restore previous timer configuration
	setitimer(ITIMER_VIRTUAL, &old_timer, NULL);
}

void preempt_enable(void)
{
	/* TODO */
	sigprocmask (SIG_UNBLOCK, &block_timer, NULL);
}

void preempt_disable(void)
{
	/* TODO */
	sigemptyset(&block_timer);
  sigaddset(&block_timer, SIGVTALRM);
	sigprocmask(SIG_BLOCK, &block_timer, NULL);
}

