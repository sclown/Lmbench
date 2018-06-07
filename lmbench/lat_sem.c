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

static void initialize(iter_t iterations, void *cookie);
static void cleanup(iter_t iterations, void *cookie);
static void doit(iter_t iterations, void *cookie);
static void writer(void *cookie);

typedef struct _state {
    pthread_t writer_tid;
    dispatch_semaphore_t sem1;
    dispatch_semaphore_t sem2;
} state_t;

void
lat_sem()
{
	state_t state;
	int parallel = 1;
	int warmup = 0;
	int repetitions = -1;
	int c;
//    char* usage = "[-P <parallelism>] [-W <warmup>] [-N <repetitions>]\n";
//
//    while (( c = getopt(ac, av, "P:W:N:")) != EOF) {
//        switch(c) {
//        case 'P':
//            parallel = atoi(optarg);
//            if (parallel <= 0) lmbench_usage(ac, av, usage);
//            break;
//        case 'W':
//            warmup = atoi(optarg);
//            break;
//        case 'N':
//            repetitions = atoi(optarg);
//            break;
//        default:
//            lmbench_usage(ac, av, usage);
//            break;
//        }
//    }
//    if (optind < ac) {
//        lmbench_usage(ac, av, usage);
//    }


	benchmp(initialize, doit, cleanup, SHORT, parallel, 
		warmup, repetitions, &state);
	micro("Semaphore latency", get_n() * 2);
}

static void
initialize(iter_t iterations, void* cookie)
{
	char	c;

	if (iterations) return;
    
    state_t * state = (state_t *)(*(void**)cookie);
    
    state->sem1 = dispatch_semaphore_create(0);
    state->sem2 = dispatch_semaphore_create(0);
    pthread_create(&(state->writer_tid), NULL, writer, cookie);

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
    dispatch_release(state->sem1);
    dispatch_release(state->sem2);
}

static void
doit(register iter_t iterations, void *cookie)
{
    state_t * state = (state_t *)(*(void**)cookie);
	while (iterations-- > 0) {
        dispatch_semaphore_wait(state->sem2, DISPATCH_TIME_FOREVER);
        dispatch_semaphore_signal(state->sem1);
	}
}

static void
writer(void* cookie)
{
    state_t * state = (state_t *)(*(void**)cookie);
    
    dispatch_semaphore_signal(state->sem2);

	for ( ;; ) {
        dispatch_semaphore_wait(state->sem1, DISPATCH_TIME_FOREVER);
        dispatch_semaphore_signal(state->sem2);
	}
}
