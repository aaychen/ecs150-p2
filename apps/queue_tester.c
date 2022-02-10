#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include <queue.h>

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

/* Test creating a queue */
void test_create(void)
{
	fprintf(stderr, "*** TEST create ***\n");

	queue_t q = queue_create();
	TEST_ASSERT(q != NULL);
	TEST_ASSERT(queue_length(q) == 0);
}

/* Enqueue/dequeue simple */
void test_queue_simple(void)
{
	int data = 3, *ptr;
	queue_t q;

	fprintf(stderr, "*** TEST queue_simple ***\n");

	q = queue_create();
	queue_enqueue(q, &data);
	queue_dequeue(q, (void**)&ptr);
	TEST_ASSERT(ptr == &data);
}

/* Test enqueueing items */
void test_enqueue(void)
{
	fprintf(stderr, "*** TEST enqueue ***\n");

	int data1 = 10, data2 = 20, data3 = 30;
	queue_t q = queue_create();

	// Null argument tests
	TEST_ASSERT(queue_enqueue(NULL, &data1) == -1);
	TEST_ASSERT(queue_enqueue(q, NULL) == -1);

	TEST_ASSERT(queue_enqueue(q, &data1) == 0);
	TEST_ASSERT(queue_length(q) == 1);
	TEST_ASSERT(queue_enqueue(q, &data2) == 0);
	TEST_ASSERT(queue_length(q) == 2);
	TEST_ASSERT(queue_enqueue(q, &data3) == 0);
	TEST_ASSERT(queue_length(q) == 3);
}

/* Test dequeueing items */
void test_dequeue(void)
{
	fprintf(stderr, "*** TEST dequeue ***\n");

	int data1 = 10, data2 = 20, data3 = 30, *ptr;
	queue_t q = queue_create();

	// Null argument tests
	TEST_ASSERT(queue_dequeue(NULL, (void**)&ptr) == -1);
	TEST_ASSERT(queue_dequeue(q, NULL) == -1);

	// Empty queue test
	TEST_ASSERT(queue_dequeue(q, (void**)&ptr) == -1);

	queue_enqueue(q, &data1);
	queue_enqueue(q, &data2);
	queue_enqueue(q, &data3);
	TEST_ASSERT(queue_dequeue(q, (void**)&ptr) == 0);
	TEST_ASSERT(ptr == &data1);
	TEST_ASSERT(*ptr == data1);
	TEST_ASSERT(queue_length(q) == 2);
	TEST_ASSERT(queue_dequeue(q, (void**)&ptr) == 0);
	TEST_ASSERT(ptr == &data2);
	TEST_ASSERT(*ptr == data2);
	TEST_ASSERT(queue_length(q) == 1);
	TEST_ASSERT(queue_dequeue(q, (void**)&ptr) == 0);
	TEST_ASSERT(ptr == &data3);
	TEST_ASSERT(*ptr == data3);
	TEST_ASSERT(queue_length(q) == 0);
}

/* Test deleting an item */
void test_delete(void)
{
	fprintf(stderr, "*** TEST delete ***\n");

	int data1 = 10, data2 = 20, data3 = 10, data4 = 40;
	queue_t q = queue_create();

	// Null arugment tests
	TEST_ASSERT(queue_delete(NULL, &data1) == -1);
	TEST_ASSERT(queue_delete(q, NULL) == -1);

	// Empty queue test
	TEST_ASSERT(queue_delete(q, &data1) == -1);

	queue_enqueue(q, &data1);
	queue_enqueue(q, &data2);
	queue_enqueue(q, &data3);
	queue_enqueue(q, &data4);
	TEST_ASSERT(queue_delete(q, &data1) == 0);
	TEST_ASSERT(queue_length(q) == 3);
	TEST_ASSERT(queue_delete(q, &data4) == 0);
	TEST_ASSERT(queue_length(q) == 2);
	TEST_ASSERT(queue_delete(q, &data1) == -1);
	TEST_ASSERT(queue_length(q) == 2);
	TEST_ASSERT(queue_delete(q, &data3) == 0);
	TEST_ASSERT(queue_length(q) == 1);
	TEST_ASSERT(queue_delete(q, &data2) == 0);
	TEST_ASSERT(queue_length(q) == 0);
}

/* Test destroying a queue */
void test_destroy(void)
{
	fprintf(stderr, "*** TEST destroy ***\n");

	int data = 10, *ptr;
	queue_t q = queue_create();

	// Null argument test
	TEST_ASSERT(queue_destroy(NULL) == -1);

	// Still have items
	queue_enqueue(q, &data);
	TEST_ASSERT(queue_destroy(q) == -1);

	// No more items
	queue_dequeue(q, (void**)&ptr);
	TEST_ASSERT(queue_destroy(q) == 0);
}

/* Callback function that increments integer items by a certain value (or delete
 * item if item is value 42). Default increment is 1 (if arg is null).
 */
static int inc_item(queue_t q, void *data, void *arg)
{
	int *a = (int*)data;
	int inc = arg != NULL ? (int)(long)arg : 1;

	if (*a == 42)
		queue_delete(q, data);
	else
		*a += inc;

	return 0;
}

/* Callback function that finds a certain item according to its value */
static int find_item(queue_t q, void *data, void *arg)
{
	int *a = (int*)data;
	int match = (int)(long)arg;
	(void)q; // unused

	if (*a == match)
		return 1;

	return 0;
}

/* Test iterating over the queue */
void test_iterate(void)
{
	fprintf(stderr, "*** TEST iterate ***\n");

	queue_t q;
	int data[] = {1, 2, 3, 4, 5, 42, 6, 7, 8, 9};
	int *ptr;

	q = queue_create(); // initalize the queue

	/* Empty queue case */
	TEST_ASSERT(queue_iterate(q, inc_item, (void*)1, NULL) == 0);
	TEST_ASSERT(queue_length(q) == 0);
	ptr = NULL;
	TEST_ASSERT(queue_iterate(q, find_item, (void*)5, (void**)&ptr) == 0);

	/* Enqueue items */
	for (size_t i = 0; i < sizeof(data) / sizeof(data[0]); i++)
		queue_enqueue(q, &data[i]);

	/* Add value '1' to every item of the queue, delete item '42' */
	queue_iterate(q, inc_item, (void*)1, NULL);
	TEST_ASSERT(data[0] == 2);
	TEST_ASSERT(queue_length(q) == 9);

	/* Find and get the item which is equal to value '5' */
	ptr = NULL; // result pointer *must* be reset first
	queue_iterate(q, find_item, (void*)5, (void**)&ptr);
	TEST_ASSERT(ptr != NULL);
	TEST_ASSERT(*ptr == 5);
	TEST_ASSERT(ptr == &data[3]);

	/* Test dequeue until empty to check delete resistance of item '42' */
	ptr = NULL;
	TEST_ASSERT(queue_dequeue(q, (void**)&ptr) == 0);
	TEST_ASSERT(queue_length(q) == 8);
	TEST_ASSERT(queue_dequeue(q, (void**)&ptr) == 0);
	TEST_ASSERT(queue_length(q) == 7);
	TEST_ASSERT(queue_dequeue(q, (void**)&ptr) == 0);
	TEST_ASSERT(queue_length(q) == 6);
	TEST_ASSERT(queue_dequeue(q, (void**)&ptr) == 0);
	TEST_ASSERT(queue_length(q) == 5);
	TEST_ASSERT(queue_dequeue(q, (void**)&ptr) == 0);
	TEST_ASSERT(queue_length(q) == 4);
	TEST_ASSERT(queue_dequeue(q, (void**)&ptr) == 0);
	TEST_ASSERT(queue_length(q) == 3);
	TEST_ASSERT(queue_dequeue(q, (void**)&ptr) == 0);
	TEST_ASSERT(queue_length(q) == 2);
	TEST_ASSERT(queue_dequeue(q, (void**)&ptr) == 0);
	TEST_ASSERT(queue_length(q) == 1);
	TEST_ASSERT(queue_dequeue(q, (void**)&ptr) == 0);
	TEST_ASSERT(queue_length(q) == 0);

}

void test_iterate_empty_queue()
{
	fprintf(stderr, "*** TEST iterate_empty_queue ***\n");

	queue_t q = queue_create();
	int *ptr = NULL;

	TEST_ASSERT(queue_iterate(q, inc_item, (void*)1, NULL) == 0);
	TEST_ASSERT(queue_length(q) == 0);
	TEST_ASSERT(queue_iterate(q, find_item, (void*)5, (void**)&ptr) == 0);
	TEST_ASSERT(queue_length(q) == 0);
}

void test_iterate_null_arg(void)
{
	fprintf(stderr, "*** TEST iterate_null_arg ***\n");

	queue_t q;
	int data[] = {1, 2, 3, 4, 5, 42, 6, 7, 8, 9};

	/* Initialize queue and enqueue items */
	q = queue_create();
	for (size_t i = 0; i < sizeof(data) / sizeof(data[0]); i++)
		queue_enqueue(q, &data[i]);

	TEST_ASSERT(queue_iterate(NULL, NULL, NULL, NULL) == -1);
	TEST_ASSERT(queue_length(q) == 10);
	TEST_ASSERT(queue_iterate(q, NULL, NULL, NULL) == -1);
	TEST_ASSERT(queue_length(q) == 10);
	TEST_ASSERT(queue_iterate(q, inc_item, NULL, NULL) == 0);
	TEST_ASSERT(data[0] == 2);
	TEST_ASSERT(queue_length(q) == 9);
}

/* Test retrieving the length of the queue */
void test_length(void)
{
	fprintf(stderr, "*** TEST length ***\n");

	// Null argument test
	TEST_ASSERT(queue_length(NULL) == -1);

	int data1 = 10, data2 = 20, data3 = 30, *ptr;
	queue_t q = queue_create();
	TEST_ASSERT(queue_length(q) == 0);

	// Check length after enqueueing
	queue_enqueue(q, &data1);
	TEST_ASSERT(queue_length(q) == 1);
	queue_enqueue(q, &data2);
	TEST_ASSERT(queue_length(q) == 2);
	queue_enqueue(q, &data3);
	TEST_ASSERT(queue_length(q) == 3);

	// Check length after dequeueing
	queue_dequeue(q, (void**)&ptr);
	TEST_ASSERT(queue_length(q) == 2);
	queue_dequeue(q, (void**)&ptr);
	TEST_ASSERT(queue_length(q) == 1);
	queue_dequeue(q, (void**)&ptr);
	TEST_ASSERT(queue_length(q) == 0);
}

int main(void)
{
	test_create();
	test_queue_simple();
	test_enqueue();
	test_dequeue();
	test_delete();
	test_destroy();
	test_iterate();
	test_iterate_empty_queue();
	test_iterate_null_arg();
	test_length();

	return 0;
}
