#include <stdio.h>
#include <numa.h>
#include <assert.h>
#include <inttypes.h>
#include <errno.h>
#include <pthread.h>
#include <signal.h>
#include <unistd.h>

struct Node
{
	uint64_t thread;
	uint64_t data;
	struct Node* link;
};

struct Queue
{
	pthread_mutex_t lock;
	struct Node *front, *rear;
};

struct Queue *initQueue(void);
int addQueue(struct Queue *q, uint64_t thread, uint64_t userVal);
int delQueue(struct Queue *q, uint64_t thread);
void walkQueue(struct Queue *q);
int walkSummary(struct Queue *q);
