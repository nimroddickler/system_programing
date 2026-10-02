#define GRID_SIZE 12
#define GENERATIONS 15

#include <pthread.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

static int grid_a[GRID_SIZE][GRID_SIZE];
static int grid_b[GRID_SIZE][GRID_SIZE];
static int (*curr)[GRID_SIZE] = grid_a;
static int (*next)[GRID_SIZE] = grid_b;

static pthread_barrier_t generation_barrier;

struct cell_arg {
    int x;
    int y;
};

static int is_alive(int x, int y) {
    if (x < 0 || x >= GRID_SIZE || y < 0 || y >= GRID_SIZE) {
        return 0;
    }
    return curr[x][y];
}

static int count_neighbors(int x, int y) {
    int count = 0;
    for (int dx = -1; dx <= 1; dx++) {
        for (int dy = -1; dy <= 1; dy++) {
            if (dx == 0 && dy == 0) {
                continue;
            }
            count += is_alive(x + dx, y + dy);
        }
    }
    return count;
}

static int next_state(int x, int y) {
    int alive = curr[x][y];
    int neighbors = count_neighbors(x, y);
    return (alive && (neighbors == 2 || neighbors == 3)) ||
           (!alive && neighbors == 3);
}

static void print_grid(int gen) {
    printf("\033[H\033[J");
    printf("Generation %d/%d\n", gen, GENERATIONS - 1);
    for (int x = 0; x < GRID_SIZE; x++) {
        for (int y = 0; y < GRID_SIZE; y++) {
            putchar(curr[x][y] ? 'X' : '.');
            putchar(' ');
        }
        putchar('\n');
    }
}

void *cell_thread(void *arg) {
    struct cell_arg *a = arg;
    int x = a->x;
    int y = a->y;

    for (int gen = 0; gen < GENERATIONS; gen++) {
        next[x][y] = next_state(x, y);

        pthread_barrier_wait(&generation_barrier);

        if (x == 0 && y == 0) {
            print_grid(gen);
            sleep(1);

            int (*tmp)[GRID_SIZE] = curr;
            curr = next;
            next = tmp;
        }

        pthread_barrier_wait(&generation_barrier);
    }

    return NULL;
}

int main(void) {
    curr[1][2] = curr[2][3] = curr[3][1] = curr[3][2] = curr[3][3] = 1;

    int err = pthread_barrier_init(&generation_barrier, NULL, GRID_SIZE * GRID_SIZE);
    if (err != 0) {
        fprintf(stderr, "pthread_barrier_init: %s\n", strerror(err));
        exit(EXIT_FAILURE);
    }

    pthread_t threads[GRID_SIZE][GRID_SIZE];
    struct cell_arg args[GRID_SIZE][GRID_SIZE];

    for (int x = 0; x < GRID_SIZE; x++) {
        for (int y = 0; y < GRID_SIZE; y++) {
            args[x][y].x = x;
            args[x][y].y = y;

            err = pthread_create(&threads[x][y], NULL, cell_thread, &args[x][y]);
            if (err != 0) {
                fprintf(stderr, "pthread_create: %s\n", strerror(err));
                exit(EXIT_FAILURE);
            }
        }
    }

    for (int x = 0; x < GRID_SIZE; x++) {
        for (int y = 0; y < GRID_SIZE; y++) {
            err = pthread_join(threads[x][y], NULL);
            if (err != 0) {
                fprintf(stderr, "pthread_join: %s\n", strerror(err));
                exit(EXIT_FAILURE);
            }
        }
    }

    err = pthread_barrier_destroy(&generation_barrier);
    if (err != 0) {
        fprintf(stderr, "pthread_barrier_destroy: %s\n", strerror(err));
        exit(EXIT_FAILURE);
    }

    return 0;
}

