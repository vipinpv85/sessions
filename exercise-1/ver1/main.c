#include "common.h"

static struct Queue *myQueue;
static uint32_t numElements = 1000;
static uint32_t numThreads = 10;
static int delstart = 0;

static void *delElements(void *userData)
{
	uint32_t myval = *(uint32_t *)userData;

	uint32_t index = numElements;
	int ret = 0;

	while (index >= 0) {
		if (delstart) {
		ret = delQueue(myQueue, myval);
		if ((ret != 0) && (ret != -EAGAIN) && (ret != -EBUSY))
			break;
		index -= 1;
		}
		else
			usleep(10);
	}
	pthread_exit(NULL);
}

static void *addElements(void *userData)
{
	uint32_t myval = *(uint32_t *)userData;

	uint32_t index = numElements;
	int32_t ret = 0;

	while (index > 0)
	{
		ret = addQueue(myQueue, myval, index);
		if ((ret != 0) && (ret != -EAGAIN))
			break;

		if (ret == 0)
			index -= 1;
	}

	pthread_exit(NULL);
}

static void *walkElements(void *user)
{
	int32_t ret = 0;

	while (1)
	{
		usleep(10000);

		ret = walkSummary(myQueue);
		if (ret == -1)
			break;
	}

	pthread_exit(NULL);
}

int main(int argc, char **argv)
{

	for (int i = 0; i < argc; i++)
		fprintf(stdout, " index-%d: (%s)\n", i, *argv);

	assert(numa_available() == 0);
	numThreads = numa_num_configured_cpus() / 4;

	myQueue = initQueue();
	assert(myQueue);
	addQueue(myQueue, 1, 0xdeadc0de);


	pthread_t test;
	pthread_t thread[1024];

	assert(pthread_create(&test, NULL, walkElements, NULL) == 0);
	for (int k = 0; k < numThreads; k++) {
		int temp = k;
		assert(pthread_create(&thread[k], NULL, addElements, (void*)&temp) == 0);
		usleep(10);
	}

	for (int k = 0; k < numThreads; k++)
		pthread_join(thread[k], NULL);

	delQueue(myQueue, 1);

	for (int k = 0; k < numThreads; k++) {
		int temp = k;
		assert(pthread_create(&thread[temp], NULL, delElements, (void*)&temp) == 0);
		usleep(10);
	}

	delstart = 1;
	for (int k = 0; k < numThreads; k++) {
		pthread_join(thread[k], NULL);
	}

	int ret = pthread_join(test, NULL);
	if ((ret ==EDEADLK) || (ret == EINVAL) || (ret == ESRCH))
		walkElements(NULL);

	return 0;
}
