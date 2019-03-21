/*
 * lat_ctx.c - context switch timer 
 *
 * usage: lat_ctx [-P parallelism] [-W <warmup>] [-N <repetitions>] [-s size] #procs [#procs....]
 *
 * Copyright (c) 1994 Larry McVoy.  Distributed under the FSF GPL with
 * additional restriction that results may published only if
 * (1) the benchmark is unmodified, and
 * (2) the version in the sccsid below is included in the report.
 * Support for this development by Sun Microsystems is gratefully acknowledged.
 */
static char    *id = "$Id$\n";

#include "bench.h"
#include <pthread.h>
#define KDEBUG
#include "kdebug.h"

#define	MAXPROC	2048
#define	CHUNK	(4<<10)
#define	TRIPS	5
#ifndef	max
#define	max(a, b)	((a) > (b) ? (a) : (b))
#endif

typedef struct _state {
    int index;
    int    process_size;
    double    overhead;
    int    procs;
    pthread_t*    tids;
    int    *p;
    void*    data;
} state_t;
static struct _state state;

static void	doit(void* args);
static int	create_pipes(int *p, int procs);
static int    create_daemons(state_t* pStateSrc);
static void	initialize_overhead(iter_t iterations, void* cookie);
static void	cleanup_overhead(iter_t iterations, void* cookie);
static void	benchmark_overhead(iter_t iterations, void* cookie);
static void	initialize(iter_t iterations, void* cookie);
static void	cleanup(iter_t iterations, void* cookie);
static void	benchmark(iter_t iterations, void* cookie);
static int process_size = 32*1024;


void
lat_ctx()
{
	int	maxprocs = 16;
	int	parallel = 1;
	int	warmup = 0;
	int	repetitions = -1;
	double	time;

	/*
	 * Need 4 byte ints.
	 */
	if (sizeof(int) != 4) {
		fprintf(stderr, "Fix sumit() in ctx.c.\n");
		exit(1);
	}

    int offset = 0;
    for(process_size = 0*1024;process_size < 10*1024*1024; process_size+=offset)
    {
    
    offset += 32*1024;
    int cores = 1;
	state.procs = cores;
    state.process_size = 32*1024;
    state.overhead = 0.0;
    state.tids = NULL;
    state.index = 0;

    KDSTART(1)
    benchmp(initialize_overhead, benchmark_overhead, cleanup_overhead, 0, parallel,
            warmup, repetitions, &state);
	if (gettime() == 0) return;
	state.overhead = gettime();
	state.overhead /= get_n();
	fprintf(stderr, "\n\"size=%dk ovr=%.2f mks\n", 
		state.process_size/1024, state.overhead);
    KDEND(1)

    state.procs = maxprocs;
    KDSTART(2)
    benchmp(initialize, benchmark, cleanup, 0, parallel,
            warmup, repetitions, &state);
    KDEND(2)
    
    time = gettime();
    time /= get_n();
    time /= state.procs;
    time -= state.overhead;
    
    fprintf(stderr, "%d %.2f mks\n", state.procs, time);

    }

}

static void
initialize_overhead(iter_t iterations, void* cookie)
{
	int procs;

	if (iterations) return;
    struct _state* pStateSrc = (struct _state*)(*(void**)cookie);
    struct _state* pState = (struct _state*)malloc(sizeof(struct _state));
    *pState = *pStateSrc;
    (*(void**)cookie) = pState;
    
	pState->tids = NULL;
	pState->p = (int*)malloc(pState->procs * (2 * sizeof(int)));
	pState->data = (pState->process_size > 0) ? malloc(pState->process_size) : NULL;
	if (!pState->p || (pState->process_size > 0 && !pState->data)) {
		perror("malloc");
		exit(1);
	}

	if (pState->data)
		bzero(pState->data, pState->process_size);

	procs = create_pipes(pState->p, pState->procs);
	if (procs < pState->procs) {
		cleanup_overhead(0, cookie);
		exit(1);
	}
}

static void
cleanup_overhead(iter_t iterations, void* cookie)
{
	int i;

	if (iterations) return;

    struct _state* pState = (struct _state*)(*(void**)cookie);
    for (i = 0; i < pState->procs; ++i) {
		close((pState->p + 2 * i)[0]);
		close((pState->p + 2 * i)[1]);
	}

	free(pState->p);
	if (pState->data) free(pState->data);

}

static void
benchmark_overhead(iter_t iterations, void* cookie)
{
    struct _state* pState = (struct _state*)(*(void**)cookie);
	int	i = 0;
	int	msg = 1;


	while (iterations-- > 0) {
        KDSTART(3)
		if (write((pState->p + 2 * i)[1], &msg, sizeof(msg)) != sizeof(msg)) {
			/* perror("read/write on pipe"); */
			exit(1);				
		}
		if (read((pState->p + 2 * i)[0], &msg, sizeof(msg)) != sizeof(msg)) {
			/* perror("read/write on pipe"); */
			exit(1);
		}
		if (++i == pState->procs) {
			i = 0;
		}
		bread(pState->data, pState->process_size);
        KDEND(3)
	}
}

static void
initialize(iter_t iterations, void* cookie)
{
    KDSIGN(7)
	int procs;

	if (iterations) return;

	initialize_overhead(iterations, cookie);
    struct _state* pState = (struct _state*)(*(void**)cookie);

	pState->tids = (pthread_t*)malloc(pState->procs * sizeof(pthread_t));
	if (pState->tids == NULL)
		exit(1);
	bzero((void*)pState->tids, pState->procs * sizeof(pthread_t));
	procs = create_daemons(pState);
    KDSIGN(5)

	if (procs < pState->procs) {
		cleanup(0, cookie);
		exit(1);
	}
}

static void
cleanup(iter_t iterations, void* cookie)
{
    KDSIGN(8)
	int i;

	if (iterations) return;

	/*
	 * Close the pipes and kill the children.
	 */
    struct _state* pState = (struct _state*)(*(void**)cookie);
	cleanup_overhead(iterations, cookie);
    for (i = 1; pState->tids && i < pState->procs; ++i) {
		if (pState->tids[i] > 0) {
            pthread_cancel(pState->tids[i]);
            pthread_join(pState->tids[i], NULL);
		}
	}
	if (pState->tids)
		free(pState->tids);
	pState->tids = NULL;
}

static void
benchmark(iter_t iterations, void* cookie)
{
    struct _state* pState = (struct _state*)(*(void**)cookie);
	int	msg;

	/*
	 * Main process - all others should be ready to roll, time the
	 * loop.
	 */
	while (iterations-- > 0) {
        KDSTART(9)
		if (write(pState->p[1], &msg, sizeof(msg)) !=
		    sizeof(msg)) {
			/* perror("read/write on pipe"); */
			exit(1);
		}
		if (read((pState->p + 2 * (pState->procs-1))[0], &msg, sizeof(msg)) != sizeof(msg)) {
			/* perror("read/write on pipe"); */
			exit(1);
		}
		bread(pState->data, pState->process_size);
        KDEND(9)
	}
}


static void
doit(void* args)
{
    state_t *stateptr = (state_t*)args;
    pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
    int i = (int)stateptr->index;
    int rd = (stateptr->p + 2 * (i - 1))[0];
    int wr = (stateptr->p + 2 * i)[1];
    int sz = stateptr->process_size;
    free(stateptr);
    stateptr = NULL;
	int	msg;
	void*	data = NULL;

	if (sz) {
		data = malloc(sz);
		if (!data) {
			perror("malloc");
			exit(3);
		}
		bzero(data, sz);
	}
	for ( ;; ) {
		if (read(rd, &msg, sizeof(msg)) != sizeof(msg)) {
			/* perror("read/write on pipe"); */
			break;
		}
        KDSTART(4)
		if (sz)
			bread(data, sz);
        KDEND(4)
		if (write(wr, &msg, sizeof(msg)) != sizeof(msg)) {
			/* perror("read/write on pipe"); */
			break;
		}
	}
    if (data) {
        free(data);
    }
}


static int
create_daemons(state_t* pStateSrc)
{
	int	i;
	int	msg;

	/*
	 * Use the pipes as a ring, and fork off a bunch of processes
	 * to pass the byte through their part of the ring.
	 *
	 * Do the sum in each process and get that time before moving on.
	 */
    
    
    
    for (i = 1; i < pStateSrc->procs; ++i) {
        state_t* pState = (state_t*)malloc(sizeof(state_t));
        *pState = *pStateSrc;
        pState->index = i;
        pthread_create(&(pState->tids[i]), NULL, doit, (void*)pState);
    }

    
	/*
	 * Go once around the loop to make sure that everyone is ready and
	 * to get the token in the pipeline.
	 */
	if (write(pStateSrc->p[1], &msg, sizeof(msg)) != sizeof(msg) ||
	    read((pStateSrc->p + 2 * (pStateSrc->procs-1))[0], &msg, sizeof(msg)) != sizeof(msg)) {
		/* perror("write/read/write on pipe"); */
		exit(1);
	}
	return pStateSrc->procs;
}

static int
create_pipes(int *p, int procs)
{
	int	i;
	/*
	 * Get a bunch of pipes.
	 */
	morefds();
    for (i = 0; i < procs; ++i) {
		if (pipe(p + (2 * i)) == -1) {
			return i;
		}
	}
	return procs;
}
