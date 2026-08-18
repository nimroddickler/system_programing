#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>

struct queue_entry {
    struct queue_entry *next;
    int value;
};

struct queue {
    struct queue_entry *head;
    struct queue_entry *tail;
    pthread_mutex_t lock;
};

void queue_init(struct queue *q) {
    q->head = NULL;
    q->tail = NULL;
    pthread_mutex_init(&q->lock, NULL);
}

void queue_destroy(struct queue *q) {
    pthread_mutex_destroy(&q->lock);
}

void queue_push(struct queue *q, struct queue_entry *entry) {
    pthread_mutex_lock(&q->lock);
    
    entry->next = NULL;
    
    if (q->tail == NULL) {
        q->head = entry;
        q->tail = entry;
    } else {
        q->tail->next = entry;
        q->tail = entry;
    }

    pthread_mutex_unlock(&q->lock);
}

struct queue_entry *queue_pop(struct queue *q) {
    pthread_mutex_lock(&q->lock);
    if (q->head == NULL) {
        pthread_mutex_unlock(&q->lock);
        return NULL;
    }
    
    struct queue_entry *entry = q->head;
    q->head = entry->next;
    if (q->head == NULL) {
        q->tail = NULL;
    }
    
    pthread_mutex_unlock(&q->lock);
    return entry;
}

void *producer(void *arg) {
    struct queue *q = arg;   /* main will pass &q */
    
    int count = (int)(random() % 10) + 1;  /* 1..10 for debugging */
    printf("producer count=%d\n", count);

    for (int i = 0; i < count; i++) {
        int value = (int)(random() % 100) + 1;  /* 1..100, never 0 */
        printf("producer value=%d\n", value);

        struct queue_entry *e = malloc(sizeof(*e));
        e->value = value;
        queue_push(q, e);
    }  
    struct queue_entry *end = malloc(sizeof(*end));
    end->value = 0;
    queue_push(q, end);
    printf("producer end\n");
    return NULL;             /* end this thread */

}

void *consumer(void *arg) {
    struct queue *q = arg;
    FILE *out = fopen("numbers.txt", "w");
    if (out == NULL) {
        perror("fopen");
        return NULL;
    }
    
    for (;;) {
        struct queue_entry *e = queue_pop(q);
        if (e == NULL) {          /* queue empty - wait and retry */
            usleep(100000);       /* 0.1 second */
            continue;
        }
        if (e->value == 0) {      /* producer said "done" */
            free(e);
            break;
        }
        fprintf(out, "%d\n", e->value);
        free(e);
    }

    fclose(out);
    return NULL;
}

int main(void) {
    struct queue q;
    pthread_t prod, cons;
    srandom((unsigned)time(NULL));

    queue_init(&q);
    
    pthread_create(&prod, NULL, producer, &q);
    pthread_create(&cons, NULL, consumer, &q);
    
    pthread_join(prod, NULL);
    pthread_join(cons, NULL);

    queue_destroy(&q);
    return 0;
}
