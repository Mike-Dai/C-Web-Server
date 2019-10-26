#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>
#include "threadpool.h"

static void* thread_routine(void *arg) {
	threadpool *tp = (threadpool*)arg;
	task_t *task;
	while (1) {
		while (!tp->task_head && !tp->shutdown) {
			pthread_cond_wait(tp->queue_ready, tp->queue_lock);
		}
		if (tp->shutdown) {
			pthread_mutex_unlock(tp->queue_lock);
			pthread_exit(NULL);
		}
		task = tp->task_head;
		tp->task_head = tp->task_head->next;
		pthread_mutex_unlock(tp->queue_lock);
		task->func(task->args);
		free(task);
	}
}


int threadpool_init(threadpool *tp) {
	tp->shutdown = 0;
	tp->maxnum = 30;
	tp->threads = (pthread_t*)malloc(sizeof(pthread_t) * tp->maxnum);
	tp->task_head = NULL;
	tp->queue_lock = (pthread_mutex_t*)malloc(sizeof(pthread_mutex_t));
	tp->queue_ready = (pthread_cond_t*)malloc(sizeof(pthread_cond_t));
	pthread_mutex_init(tp->queue_lock, NULL);
	pthread_cond_init(tp->queue_ready, NULL);
	for (int i = 0; i < tp->maxnum; i++) {
		pthread_create(&tp->threads[i], NULL, thread_routine, (void*)tp);
	}
	return 0;
}

void threadpool_destroy(threadpool *tp) {
	tp->shutdown = 1;
	pthread_mutex_lock(tp->queue_lock);
	pthread_cond_broadcast(tp->queue_ready);
	pthread_mutex_unlock(tp->queue_lock);
	for (int i = 0; i < tp->maxnum; i++) {
		pthread_join(tp->threads[i], NULL);
	}
	free(tp->threads);
	task_t *p;
	while (tp->task_head) {
		p = tp->task_head;
		tp->task_head = tp->task_head->next;
		free(p);
	}
	pthread_mutex_destroy(tp->queue_lock);
	pthread_cond_destroy(tp->queue_ready);
}

int add_worker(threadpool *tp, void*(*func)(void*), void *args) {
	task_t *task = (task_t*)malloc(sizeof(task_t));
	task->func = func;
	task->args = args;
	task->next = NULL;
	pthread_mutex_lock(tp->queue_lock);
	task_t *p = tp->task_head;
	if (!p) {
		tp->task_head = task;
	}
	else {
		while (p->next != NULL) {
			p = p->next;
		}
		p->next = task;
	}
	pthread_cond_signal(tp->queue_ready);
	pthread_mutex_unlock(tp->queue_lock);
	return 0;
}

