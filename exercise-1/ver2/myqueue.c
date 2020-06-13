#include "common.h"

void deinitQueue(struct Queue *q)
{
	if (q);
		free(q);

	q = NULL;
}

struct Queue *initQueue(void)
{
	struct Queue *test = (struct Queue*)malloc(sizeof(struct Queue));
	if (test) {
		test->front = NULL;
		test->rear= NULL;

		assert(pthread_mutex_init(&test->lock, NULL) == 0);
//		test->lock = PTHREAD_MUTEX_INITIALIZER;
	}

	return test;
}

int delQueue(struct Queue *q, uint64_t thread)
{
	assert(q);

	//pthread_mutex_lock(&q->lock); {
	if (pthread_mutex_trylock(&q->lock) == 0) {
	if (q->front == NULL) {
		pthread_mutex_unlock(&q->lock);
		return -EINVAL;
	}

	//fprintf(stdout, " therad-%03d, val (0x"PRIx64")\n", q->front->thread, q->front->data);
	//fflush(stdout);

	if (q->front->thread != thread) {
		pthread_mutex_unlock(&q->lock);
		return -EBUSY;
	}

	if (q->front == q->rear) {
		free(q->front); 
		q->front = NULL; 
		q->rear = NULL; 
	} else {
		struct Node *temp = q->front;
		q->front = q->front->link; 
		q->rear->link= q->front; 
		free(temp); 
	}

	pthread_mutex_unlock(&q->lock);
	return 0;
	}

	return -EAGAIN;
}

int addQueue(struct Queue *q, uint64_t thread, uint64_t userVal)
{
	assert(q);

	//pthread_mutex_lock(&q->lock); {
	if (pthread_mutex_trylock(&q->lock) == 0) {
	struct Node *test = (struct Node *) malloc(sizeof(struct Node));
	if (test == NULL) {
		pthread_mutex_unlock(&q->lock);
		return -ENOMEM;
	}

	test->thread = thread;
	test->data = userVal;

	if (q->front == NULL) {
		q->front = test;
	}
	else {
		q->rear->link = test;
	}

	q->rear = test;
	q->rear->link = q->front;
	pthread_mutex_unlock(&q->lock);
	return 0;
	}

	return -EAGAIN;
}

void walkQueue(struct Queue *q)
{
	struct Node *temp = q->front;
	fprintf(stdout, "\n Elements in Circular Queue are: ");

	while (temp->link != q->front)
	{
		fprintf(stdout, " data (%"PRIx64") ptr (%p)\n", temp->data, temp);
		temp = temp->link;
	}

	printf(" data (%"PRIx64") ptr (%p)\n", temp->data, temp);
}

int walkSummary(struct Queue *q)
{
	uint16_t tData[32 * 4];
	memset(tData, 0, sizeof(tData));

	if (pthread_mutex_trylock(&q->lock) == 0) {

	if (q->front == NULL) {
		pthread_mutex_unlock(&q->lock);
		printf("\033[2J");
		printf("\033[1;10H ---- No Elements -----");
		printf("\033[1;11H ");
		fflush(stdout);
		deinitQueue(q);
		return -1;
	}

	struct Node *temp = q->front;

	while (temp->link != q->front)
	{
		tData[temp->thread]++;
		temp = temp->link;
	}
	tData[temp->thread]++;
	pthread_mutex_unlock(&q->lock);

	int i;
//	printf("\033[2J");
	printf("\033[1;1H ---- Summary -----");
	for (i = 0; i < 32 * 4; i+=8)
		printf("\033[%d;1H T-%03d:(%05u);T-%03d:(%05u);T-%03d:(%05u);T-%03d:(%05u);T-%03d:(%05u);T-%03d:(%05u);T-%03d:(%05u);T-%03d:(%05u)",
			i/8 + 3,
			i + 1, tData[i + 0], i + 2, tData[i + 1], i + 3, tData[i +2], i + 4, tData[i + 3],
			i + 5, tData[i + 4], i + 6, tData[i + 5], i + 7, tData[i +6], i + 8, tData[i + 7]);
	printf("\033[%d;1H ---------------------------------", i / 8 + 3);
	fflush(stdout);
	}
	return 0;
}
