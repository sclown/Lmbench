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
static char    *id = "$Id$\n";

#include "bench.h"
#include <sys/sem.h>
#include <dispatch/dispatch.h>
#include <pthread.h>
#include <sys/kdebug_signpost.h>

static void initialize(iter_t iterations, void *cookie);
static void cleanup(iter_t iterations, void *cookie);
static void benchmark(iter_t iterations, void *cookie);
static void benchmark_over(iter_t iterations, void *cookie);
static void writer(void *cookie);

static const int threads_count = 16;
static const int buff_size = 10*1024*1024;
static void*    data;

typedef struct _state {
    pthread_t writer_tid[threads_count];
} state_t;

static dispatch_semaphore_t sem1;
static dispatch_semaphore_t sem2;


void
lat_ctx_random()
{
    state_t state;
    int parallel = 1;
    int warmup = 0;
    int repetitions = -1;
    
    int i=0;
    sem1 = dispatch_semaphore_create(0);
    sem2 = dispatch_semaphore_create(0);
    data = malloc(buff_size*threads_count);
    for(i =0; i < threads_count; ++i) {
        pthread_create(&(state.writer_tid[i]), NULL, writer, i);
    }

    double overhead = 0.0;
    double result = 0.0;
    benchmp(initialize, benchmark_over, cleanup, 0, parallel,
            warmup, repetitions, &state);
    overhead = gettime();
    overhead /= get_n();
    int count_over = get_n();
    micro("Overhead", get_n());
    benchmp(initialize, benchmark, cleanup, 0, parallel,
            warmup, repetitions, &state);
    result = gettime();
    result /= get_n();
    result /= threads_count/4;
    result -= overhead;
    int count = get_n();
    fprintf(stderr, "\n\"overhead=%.2f create-join=%.2f\n", overhead, result);
}


static void
initialize(iter_t iterations, void* cookie)
{
}

static void
cleanup(iter_t iterations, void* cookie)
{
}

static void
benchmark_over(register iter_t iterations, void *cookie)
{
    while (iterations-- > 0) {
        kdebug_signpost_start(1,0,0,0,1);
        dispatch_semaphore_signal(sem1);
        dispatch_semaphore_signal(sem1);
        dispatch_semaphore_wait(sem2, DISPATCH_TIME_FOREVER);
        dispatch_semaphore_wait(sem2, DISPATCH_TIME_FOREVER);
        kdebug_signpost_end(1,0,0,0,1);
    }
}

static void
benchmark(register iter_t iterations, void *cookie)
{
    while (iterations-- > 0) {
        int i=0;
        kdebug_signpost_start(2,0,0,0,2);
        for(i =0; i < threads_count; ++i) {
            dispatch_semaphore_signal(sem1);
        }
        for(i =0; i < threads_count; ++i) {
            dispatch_semaphore_wait(sem2, DISPATCH_TIME_FOREVER);
        }
        kdebug_signpost_end(2,0,0,0,2);
    }
}

static void
writer(void* cookie)
{
    int index = (int)cookie;
    while (true) {
        if (dispatch_semaphore_wait(sem1, DISPATCH_TIME_FOREVER)) {
            break;
        }
        bread(data+(buff_size*index), buff_size);
        dispatch_semaphore_signal(sem2);
    }
}

