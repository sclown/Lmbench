/*
 * lat_sem.c - semaphore test
 *
 * usage: lat_sem [-P <parallelism>] [-W <warmup>] [-N <repetitions>]
 *
 * Copyright (c) 2000 Carl Staelin.
 * Copyright (c) 1994 Larry McVoy.  Distributed under the FSF GPL with
 * additional restriction that results may published only if
 * (1) the benchmark is unmodified, and
 * (2) the version in the sccsid below is included in the report.
 * Support for this development by Sun Microsystems is gratefully acknowledged.
 */
static char	*id = "$Id$\n";

#include "bench.h"
#include <sys/sem.h>
#include <dispatch/dispatch.h>
#include <pthread.h>

#define KDEBUG
#include "kdebug.h"


static void initialize(iter_t iterations, void *cookie);
static void cleanup(iter_t iterations, void *cookie);
static void benchmark(iter_t iterations, void *cookie);
static void benchmark_over(iter_t iterations, void *cookie);
static void writer(void *cookie);

static const int buffer_size = 1*1024*1024;

typedef struct _state {
    pthread_t writer_tid;
    void*    data;
} state_t;

void
lat_thread()
{
	state_t state;
	int parallel = 1;
	int warmup = 0;
	int repetitions = -1;

    double overhead = 0.0;
    double result = 0.0;
    benchmp(initialize, benchmark_over, cleanup, 0, parallel,
            warmup, repetitions, &state);
    overhead = gettime();
    overhead /= get_n();
    micro("Overhead", get_n());
	benchmp(initialize, benchmark, cleanup, 0, parallel,
		warmup, repetitions, &state);
    result = gettime();
    result /= get_n();
    result -= overhead;
    fprintf(stderr, "\n\"overhead=%.2f create-join=%.2f\n", overhead, result);
}


static void
initialize(iter_t iterations, void* cookie)
{
	if (iterations) return;
    state_t * state = (state_t *)(*(void**)cookie);
    state->data = malloc(buffer_size);
    if (!state->data) {
        perror("malloc");
        exit(1);
    }

}

static void
cleanup(iter_t iterations, void* cookie)
{

	if (iterations) return;

    state_t * state = (state_t *)(*(void**)cookie);
    if (state->writer_tid > 0) {
        pthread_cancel(state->writer_tid);
        pthread_join(state->writer_tid, NULL);
    }
    free(state->data);
}

static void
benchmark_over(register iter_t iterations, void *cookie)
{
//    state_t * state = (state_t *)(*(void**)cookie);
	while (iterations-- > 0) {
        writer(cookie);
	}
}

static void
benchmark(register iter_t iterations, void *cookie)
{
    state_t * state = (state_t *)(*(void**)cookie);
    while (iterations-- > 0) {
        KDSTART(1)
        pthread_create(&(state->writer_tid), NULL, writer, cookie);
        KDEND(1)
        KDSTART(2)
        pthread_join(state->writer_tid, NULL);
        KDEND(2)
    }
}

static void
writer(void* cookie)
{
    KDSTART(3)
    state_t * state = (state_t *)(*(void**)cookie);
    bread(state->data, buffer_size);
    KDEND(3)
}
