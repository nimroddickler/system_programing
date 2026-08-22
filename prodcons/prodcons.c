#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

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
    int err = pthread_mutex_init(&q->lock, NULL);
    if (err != 0) {
        fprintf(stderr, "pthread_mutex_init: %s\n", strerror(err));
        exit(EXIT_FAILURE);
    }
}

void queue_destroy(struct queue *q) {
    int err = pthread_mutex_destroy(&q->lock);
    if (err != 0) {
        fprintf(stderr, "pthread_mutex_destroy: %s\n", strerror(err));
        exit(EXIT_FAILURE);
    }
}

void queue_push(struct queue *q, struct queue_entry *entry) {
    int err = pthread_mutex_lock(&q->lock);
    if (err != 0) {
        fprintf(stderr, "pthread_mutex_lock: %s\n", strerror(err));
        exit(EXIT_FAILURE);
    }

    entry->next = NULL;

    if (q->tail == NULL) {
        q->head = entry;
        q->tail = entry;
    } else {
        q->tail->next = entry;
        q->tail = entry;
    }

    err = pthread_mutex_unlock(&q->lock);
    if (err != 0) {
        fprintf(stderr, "pthread_mutex_unlock: %s\n", strerror(err));
        exit(EXIT_FAILURE);
    }
}

struct queue_entry *queue_pop(struct queue *q) {
    int err = pthread_mutex_lock(&q->lock);
    if (err != 0) {
        fprintf(stderr, "pthread_mutex_lock: %s\n", strerror(err));
        exit(EXIT_FAILURE);
    }

    if (q->head == NULL) {
        err = pthread_mutex_unlock(&q->lock);
        if (err != 0) {
            fprintf(stderr, "pthread_mutex_unlock: %s\n", strerror(err));
            exit(EXIT_FAILURE);
        }
        return NULL;
    }

    struct queue_entry *entry = q->head;
    q->head = entry->next;
    if (q->head == NULL) {
        q->tail = NULL;
    }

    err = pthread_mutex_unlock(&q->lock);
    if (err != 0) {
        fprintf(stderr, "pthread_mutex_unlock: %s\n", strerror(err));
        exit(EXIT_FAILURE);
    }
    return entry;
}

void *producer(void *arg) {
    struct queue *q = arg; /* main will pass &q */

    int count = (int)(random() % 10) + 1; /* 1..10 for debugging */
    printf("producer count=%d\n", count);

    for (int i = 0; i < count; i++) {
        int value = (int)(random() % 100) + 1; /* 1..100, never 0 */
        printf("producer value=%d\n", value);

        struct queue_entry *e = malloc(sizeof(*e));
        if (e == NULL) {
            perror("malloc");
            exit(EXIT_FAILURE);
        }
        e->value = value;
        queue_push(q, e);
    }
    struct queue_entry *end = malloc(sizeof(*end));
    if (end == NULL) {
        perror("malloc");
        exit(EXIT_FAILURE);
    }
    end->value = 0;
    queue_push(q, end);
    printf("producer end\n");
    return NULL; /* end this thread */
}

void *consumer(void *arg) {
    struct queue *q = arg;
    FILE *out = fopen("numbers.txt", "w");
    if (out == NULL) {
        perror("fopen");
        exit(EXIT_FAILURE);
    }

    for (;;) {
        struct queue_entry *e = queue_pop(q);
        if (e == NULL) { /* queue empty - wait and retry */
            usleep(100000); /* 0.1 second */
            continue;
        }
        if (e->value == 0) { /* producer said "done" */
            free(e);
            break;
        }
        fprintf(out, "%d\n", e->value);
        free(e);
    }

    if (fclose(out) != 0) {
        perror("fclose");
        exit(EXIT_FAILURE);
    }
    return NULL;
}

int main(void) {
    struct queue q;
    pthread_t prod, cons;
    srandom((unsigned)time(NULL));

    queue_init(&q);

    int err = pthread_create(&prod, NULL, producer, &q);
    if (err != 0) {
        fprintf(stderr, "pthread_create: %s\n", strerror(err));
        exit(EXIT_FAILURE);
    }
    err = pthread_create(&cons, NULL, consumer, &q);
    if (err != 0) {
        fprintf(stderr, "pthread_create: %s\n", strerror(err));
        exit(EXIT_FAILURE);
    }

    err = pthread_join(prod, NULL);
    if (err != 0) {
        fprintf(stderr, "pthread_join: %s\n", strerror(err));
        exit(EXIT_FAILURE);
    }
    err = pthread_join(cons, NULL);
    if (err != 0) {
        fprintf(stderr, "pthread_join: %s\n", strerror(err));
        exit(EXIT_FAILURE);
    }

    queue_destroy(&q);
    return 0;
}
