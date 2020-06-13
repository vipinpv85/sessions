#include "common.h"

#include <signal.h>
#include <emmintrin.h>

static struct Queue *myQueue;
static uint32_t numElements = 9999;
static uint32_t numThreads = 10;
static int addstart = 0;
static int delstart = 0;
static pthread_t test;
static pthread_t thread[1024];
static sigset_t signal_mask;

static void *delElements(void *userData)
{
	uint32_t myval = *(uint32_t *)userData;

	uint32_t index = numElements;
	int ret = 0;

	while (index >= 0) {
		if (__sync_fetch_and_and(&delstart, 3) == 3) {
			pthread_exit(NULL);
			return NULL;
		}

		if (__sync_fetch_and_and(&delstart, 1)) {
		ret = delQueue(myQueue, myval);
		if ((ret != 0) && (ret != -EAGAIN) && (ret != -EBUSY))
			break;
		index -= 1;
		}
		else
			usleep(1);
	}
	pthread_exit(NULL);
}

static void *addElements(void *userData)
{
	uint32_t myval = *(uint32_t *)userData;
	_mm_mfence();

	uint32_t index = numElements;
	int32_t ret = 0;

	while (index > 0)
	{
		if (__sync_fetch_and_and(&addstart, 3) == 3) {
			pthread_exit(NULL);
			return NULL;
		}

		if (__sync_fetch_and_and(&addstart, 1)) {
		ret = addQueue(myQueue, myval, index);
		if ((ret != 0) && (ret != -EAGAIN))
			break;

		if (ret == 0)
			index -= 1;
		}
		else
			usleep(1);
	}

	pthread_exit(NULL);
}

static void *walkElements(void *user)
{
	int32_t ret = 0;

	while (1)
	{
		if (__sync_fetch_and_and(&delstart, 3) == 3) {
			pthread_exit(NULL);
		}
		usleep(1000);

		ret = walkSummary(myQueue);
		if (ret == -1)
			break;
	}

	pthread_exit(NULL);
}

static void
signal_handler(int signum)
{
	__sync_fetch_and_or(&addstart, 3);
	__sync_fetch_and_or(&delstart, 3);

	printf(" addstart %d delstart %d\n", addstart, delstart);

//	for (int k = 0; k < numThreads; k++)
//		printf(" cancel (%ld), ret (%d)\n", thread[k],pthread_cancel(thread[k]));

//	printf(" cancel walk thread (%$ld), ret (%d)\n", test, pthread_cancel(test));
}

void *signal_thread (void *arg)
{
    int       sig_caught;    /* signal caught       */
    int       rc;            /* returned code       */


    rc = sigwait (&signal_mask, &sig_caught);
    if (rc != 0) {
        /* handle error */
    }
    switch (sig_caught)
    {
    case SIGINT:     /* process SIGINT  */
    case SIGTERM:    /* process SIGTERM */
	__sync_fetch_and_or(&addstart, 3);
//	__sync_fetch_and_or(&delstart, 3);
        break;
    default:         /* should normally not happen */
        fprintf (stderr, "\nUnexpected signal %d\n", sig_caught);
        break;
    }
}

int main(int argc, char **argv)
{
#if 0
	signal(SIGINT, signal_handler);
	signal(SIGTERM, signal_handler);
#else
	sigemptyset (&signal_mask);
	sigaddset (&signal_mask, SIGINT);
	sigaddset (&signal_mask, SIGTERM);
	assert(pthread_sigmask (SIG_BLOCK, &signal_mask, NULL) == 0);

	pthread_t sig_thr_id;
	assert(pthread_create(&sig_thr_id, NULL, signal_thread, NULL) == 0);
#endif

	for (int i = 0; i < argc; i++)
		fprintf(stdout, " index-%d: (%s)\n", i, *argv);

	printf("\033[2J");
	assert(numa_available() == 0);
	numThreads = numa_num_configured_cpus();

	myQueue = initQueue();
	assert(myQueue);
	addQueue(myQueue, 1, 0xdeadc0de);


	assert(pthread_create(&test, NULL, walkElements, NULL) == 0);

	for (int k = 0; k < numThreads; k++) {
		int temp = k;
		assert(pthread_create(&thread[k], NULL, addElements, (void*)&temp) == 0);
		usleep(100);
	}
	__sync_fetch_and_or(&addstart, 1);

	for (int k = 0; k < numThreads; k++)
		pthread_join(thread[k], NULL);

	delQueue(myQueue, 1);

	for (int k = 0; k < numThreads; k++) {
		int temp = k;
		assert(pthread_create(&thread[temp], NULL, delElements, (void*)&temp) == 0);
		usleep(100);
	}

	__sync_fetch_and_or(&delstart, 1);
	for (int k = 0; k < numThreads; k++) {
		pthread_join(thread[k], NULL);
	}

	int ret = pthread_join(test, NULL);

	deinitQueue(myQueue);
	return 0;
}
