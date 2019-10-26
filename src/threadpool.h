#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>

typedef struct task_s{
	void* (*func)(void*);
	void  *args;
	struct task_s *next;
}task_t;

typedef struct threadpool_s{
	int shutdown;
	int maxnum;
	pthread_t *threads;
	task_t *task_head;
	pthread_mutex_t *queue_lock;
	pthread_cond_t  *queue_ready;

}threadpool;

extern int threadpool_init(threadpool *tp);
extern void threadpool_destroy(threadpool *tp);
extern int add_worker(threadpool *tp, void*(*func)(void*), void *args);