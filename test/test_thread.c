#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define MAX_THREADS 5
#define QUEUE_SIZE 10

typedef struct {
    void (*function)(void *arg);
    void *arg;
} task_t;

typedef struct {
    task_t task_queue[QUEUE_SIZE];
    int queue_front;
    int queue_rear;
    int count;
    pthread_mutex_t lock;
    pthread_cond_t notify;
    pthread_t threads[MAX_THREADS];
    int shutdown;
} threadpool_t;

void *thread_do_work(void *arg);

threadpool_t *threadpool_create();
void threadpool_add(threadpool_t *pool, void (*function)(void *), void *arg);
void threadpool_destroy(threadpool_t *pool);

void *thread_do_work(void *arg) {
    threadpool_t *pool = (threadpool_t *)arg;
    while (1) {
        pthread_mutex_lock(&(pool->lock));
        while ((pool->count == 0) && (!pool->shutdown)) {
            pthread_cond_wait(&(pool->notify), &(pool->lock));
        }
        if (pool->shutdown) {
            break;
        }
        task_t task = pool->task_queue[pool->queue_front];
        pool->queue_front = (pool->queue_front + 1) % QUEUE_SIZE;
        pool->count--;
        pthread_mutex_unlock(&(pool->lock));

        (*(task.function))(task.arg);
    }
    pthread_mutex_unlock(&(pool->lock));
    pthread_exit(NULL);
    return NULL;
}

threadpool_t *threadpool_create() {
    threadpool_t *pool = (threadpool_t *)malloc(sizeof(threadpool_t));
    pool->queue_front = 0;
    pool->queue_rear = 0;
    pool->count = 0;
    pool->shutdown = 0;
    pthread_mutex_init(&(pool->lock), NULL);
    pthread_cond_init(&(pool->notify), NULL);
    for (int i = 0; i < MAX_THREADS; i++) {
        pthread_create(&(pool->threads[i]), NULL, thread_do_work, (void *)pool);
    }
    return pool;
}

void threadpool_add(threadpool_t *pool, void (*function)(void *), void *arg) {
    pthread_mutex_lock(&(pool->lock));
    pool->task_queue[pool->queue_rear].function = function;
    pool->task_queue[pool->queue_rear].arg = arg;
    pool->queue_rear = (pool->queue_rear + 1) % QUEUE_SIZE;
    pool->count++;
    pthread_cond_signal(&(pool->notify));
    pthread_mutex_unlock(&(pool->lock));
}

void threadpool_destroy(threadpool_t *pool) {
    pthread_mutex_lock(&(pool->lock));
    pool->shutdown = 1;
    pthread_cond_broadcast(&(pool->notify));
    pthread_mutex_unlock(&(pool->lock));
    for (int i = 0; i < MAX_THREADS; i++) {
        pthread_join(pool->threads[i], NULL);
    }
    pthread_mutex_destroy(&(pool->lock));
    pthread_cond_destroy(&(pool->notify));
    free(pool);
}

void task_function(void *arg) {
    int *num = (int *)arg;
    printf("Thread %lu is working on task %d\n", pthread_self(), *num);
    sleep(1);
    free(num);
}

int main() {
    threadpool_t *pool = threadpool_create();
    for (int i = 0; i < 20; i++) {
        int *num = (int *)malloc(sizeof(int));
        *num = i;
        threadpool_add(pool, task_function, num);
    }
    sleep(5);
    threadpool_destroy(pool);
    return 0;
}
