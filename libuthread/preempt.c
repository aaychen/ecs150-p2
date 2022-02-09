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
#define INTERVAL (1.0 / HZ * 1000) /* number of milliseconds to go off */

struct sigaction new_act;
struct sigaction old_act; // to store previous signal action
struct itimerval new_timer;
struct itimerval old_timer; // to store previous timer configuration
sigset_t block_timer_mask;

void timer_handler(int signum){
	(void)signum;
	printf("%s:%d: Inside timer_handler()\n", __FILE__, __LINE__);
	uthread_yield();
}

void preempt_start(void)
{
	/* TODO */
	// Set up sigaction
	new_act.sa_handler = timer_handler; // set the handler
	sigemptyset(&new_act.sa_mask); // no signal is blocked
	new_act.sa_flags = 0; // no flag
	sigaction(SIGVTALRM, &new_act, &old_act);
	
	// Initialize mask to block SIGVTALRM
	sigemptyset(&block_timer_mask);
	sigaddset(&block_timer_mask, SIGVTALRM);

	// Configure timer
	// First timer interrupt after 10 msec
	new_timer.it_value.tv_sec = 0;
	new_timer.it_value.tv_usec = INTERVAL * 1000;
	// Successive timer interrupts every 10 msec after that
	new_timer.it_interval = new_timer.it_value;
	setitimer(ITIMER_VIRTUAL, &new_timer, &old_timer);
	printf("%s:%d: Inside preempt_start()\n", __FILE__, __LINE__);
}

void preempt_stop(void)
{
	/* TODO */
	// Restore previous signal action
	sigaction(SIGVTALRM, &old_act, NULL);
	// Restore previous timer configuration
	setitimer(ITIMER_VIRTUAL, &old_timer, NULL);
	printf("%s:%d: Inside preempt_stop()\n", __FILE__, __LINE__);
}

void preempt_enable(void)
{
	/* TODO */
	sigprocmask(SIG_UNBLOCK, &block_timer_mask, NULL);
	printf("%s:%d: Inside preempt_enable()\n", __FILE__, __LINE__);
}

void preempt_disable(void)
{
	/* TODO */
	sigprocmask(SIG_BLOCK, &block_timer_mask, NULL);
	printf("%s:%d: Inside preempt_disable()\n", __FILE__, __LINE__);
}

