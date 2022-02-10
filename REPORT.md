# LIBUTHREAD: User-Level Thread Library

## Summary

This library, `libuthread`, is a lightweight user-level thread library for
Linux. It offers applications a comprehensive interface for creating and running
independent threads concurrently. The library has the following features:

1. Create new threads
2. Schedule the execution of threads in a round-robin fashion
3. Provide a synchronization mechanism for threads to join other threads
4. Provide an interrupt-based scheduler

## Implementation

The library's implementation is divided into three stages:

1. Implementing the API for a FIFO queue that will be used to schedule threads
   later
2. Implementing the API for the uthread library, which is the core component
   of this thread library and is responsible for thread management
3. Implementing the preemption API to periodically interrupt the current thread
   and force it to yield

### Queue API

We used a doubly linked list to construct our FIFO queue because all operations
must be O(1) except for the iterate and delete operations. Not only does a
doubly linked list help us meet the constraint, but it also makes adding and
removing items from the queue easier and more efficient. We have an internal
node structure to support our queue data structure, which has three components:
the queue length, a head node to keep track of the oldest item, and a tail node
to maintain the newest item. Our node structure is similar to that of a typical
node in a doubly linked list, with three fields: the node's data value, a
reference to the previous node, and a reference to the next node in the queue.

#### Queue API Testing
The source code related to further testing the queue API (aside from the example
tests provided to us) can be found in `apps/queue_tester.c`. The following
operations were tested: `create`, `enqueue`, `dequeue`, `delete`, `destroy`,
`iterate`, and `length`. For each operation, we tested the operations against
typical use cases, null argument cases, and edge cases where the queue is empty.

### uthread API
The uthread API uses the queue API and operations. To create the uthread
library, we used 3 queues to hold threads of the following statuses: `READY`,
`BLOCKED`, and `ZOMBIE`. We also represented the threads by defining a thread
control block (TCB) data structure called `struct tcb` which we define as `tcb`.
The currently active and running thread is noted by using a global variable so
that we are easily able to refer to the data associated with this thread.

#### Representation of Threads
An instance of `struct tcb` holds the following information:
- TID (thread identifier)
- Thread state
- Thread context
- Stack for local variables associated with the thread
- Return value if the thread has finished executing
- TID of the thread that joined it (if this is equal to its own TID, then it has
  not been joined yet)

#### `READY` Queue
This queue contains threads that are ready to be executed. When creating a new
thread, the new thread will be enqueued here. When a thread yields, the queue
will be dequeued to get the oldest thread and we will context switch to it. The
thread that yielded will be added to the end of the queue. However, if a thread
yields and the queue is empty, we do not switch contexts and continue running
the thread. Threads that are blocked or dead will never exist in this queue.

#### `BLOCKED` Queue
This queue contains threads that are blocked and cannot be yielded to yet. This
includes threads that have joined a threadX, but threadX has not finished
executing yet. The threads here only become unblocked if the thread that they
join has finished executing. When unblocked, they will be added to the end of
the `READY` queue.

The `BLOCKED` queue is never dequeued since we are not concerned with the oldest
thread that was blocked. So, when unblocking a thread, we use the iterate
operation in the queue API to find the calling thread, delete it from this
queue, change its status to `READY`, and enqueue it into the `READY` queue.

#### `ZOMBIE` Queue
This queue contains threads that are dead, meaning that the thread has finished
executing but its return value has not been collected yet. The threads here will
only be deleted from the queue if it has been joined by another thread and its
return value is collected. Once a thread in this queue is collected, all data
associated with it are deallocated and destroyed. This is queue also never
dequeued from since we are not concerned with the oldest dead thread; instead,
we use the delete function from the queue API.

#### `uthread` API Testing
The source code related to testing the uthread API can be found in
`apps/uthread_tester.c`. We tested creations/executions of single threads and
multiple threads, a single thread being joined by multiple threads, multiple
threads joining a single thread, and joining a dead thread. In all the test
cases, preemption is disabled to test against specific output.

### Preemption

To implement preemption, we first set up an alarm that sends out a `SIGVTALRM`
signal at a frequency of 100Hz or every 10,000 microseconds. Then, we created a
signal handler that functions as an alarm interrupt handler, forcing the
currently running thread to yield so that another thread can be scheduled in its
place. The signal can be enabled or disabled depending on an application's
decision when starting the multithreading phase. In addition, there are critical
sections within the uthread library where preemption is disabled.

#### Preemption Testing

The source code related to testing preemption can be found in
`apps/test_preemption.c`. In addition to the main one, we scheduled two more
threads, one of which includes an infinite while-loop. This allows us to verify
that the alarm is set up correctly and that the infinite loop thread is
interrupted, allowing the other threads to make progress. If preemption were
disabled, running the test executable would never end. 

---
## References
Aside from Professor Porquet-Lupine’s lectures and slides, below are additional
sources that we referenced to complete this project.

### Queue
- [Data structures: Introduction to Doubly Linked
  List](https://youtu.be/JdQeNxWCguQ)

### Threads
- [Allocating memory for a
  struct](https://stackoverflow.com/questions/8762278/are-mallocsizeofstruct-a-and-mallocsizeofstruct-a-the-same)

### Preemption
- [GNU C Library: Specifying Signal
  Actions](https://www.gnu.org/software/libc/manual/html_mono/libc.html#Signal-Actions)
- [GNU C Library: Setting an
  Alarm](https://www.gnu.org/software/libc/manual/html_mono/libc.html#Setting-an-Alarm)
- [GNU C Library: Blocking
  Signals](https://www.gnu.org/software/libc/manual/html_mono/libc.html#Blocking-Signals)
- [Difference between Preemptive and Cooperative Multitasking](https://www.geeksforgeeks.org/difference-between-preemptive-and-cooperative-multitasking/)