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
#include <sys/kdebug_signpost.h>

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
    int    **p;
    void*    data;
} state_t;
static struct _state state;

static void	doit(void* args);
static int	create_pipes(int **p, int procs);
static int    create_daemons(state_t* pStateSrc);
static void	initialize_overhead(iter_t iterations, void* cookie);
static void	cleanup_overhead(iter_t iterations, void* cookie);
static void	benchmark_overhead(iter_t iterations, void* cookie);
static void	initialize(iter_t iterations, void* cookie);
static void	cleanup(iter_t iterations, void* cookie);
static void	benchmark(iter_t iterations, void* cookie);
static int process_size = 5*1024*1024;


void
lat_ctx()
{
	int	i, maxprocs = 16;
	int	c;
	int	parallel = 1;
	int	warmup = 0;
	int	repetitions = -1;
	char *usage = "[-P <parallelism>] [-W <warmup>] [-N <repetitions>] [-s kbytes] processes [processes ...]\n";
	double	time;

	/*
	 * Need 4 byte ints.
	 */
	if (sizeof(int) != 4) {
		fprintf(stderr, "Fix sumit() in ctx.c.\n");
		exit(1);
	}

	state.process_size = process_size;
	state.overhead = 0.0;
	state.tids = NULL;
    state.index = 0;

	/*
	 * If they specified a context size, or parallelism level, get them.
	 */
//    while (( c = getopt(ac, av, "s:P:W:N:")) != EOF) {
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
//        case 's':
//            state.process_size = atoi(optarg) * 1024;
//            break;
//        default:
//            lmbench_usage(ac, av, usage);
//            break;
//        }
//    }
//
//    if (optind > ac - 1)
//        lmbench_usage(ac, av, usage);

	/* compute pipe + sumit overhead */
//    for (i = optind; i < ac; ++i) {
//        state.procs = atoi(av[i]);
//        if (state.procs > maxprocs)
//            maxprocs = state.procs;
//    }
    int cores = 2;
	state.procs = cores;
    kdebug_signpost(1,0,0,0,0);
    benchmp(initialize, benchmark, cleanup, 0, parallel,
            warmup, repetitions, &state);
	if (gettime() == 0) return;
	state.overhead = gettime();
	state.overhead /= get_n();
	fprintf(stderr, "\n\"size=%dk ovr=%.2f\n", 
		state.process_size/1024, state.overhead);
    kdebug_signpost(2,0,0,0,0);

	/* compute the context switch cost for N processes */
//    for (i = optind; i < ac; ++i) {
    
		state.procs = maxprocs;
		benchmp(initialize, benchmark, cleanup, 0, parallel, 
			warmup, repetitions, &state);

		time = gettime();
		time /= get_n();
		time /= state.procs/cores;
		time -= state.overhead;

		if (time > 0.0)
			fprintf(stderr, "%d %.2f\n", state.procs, time);
    kdebug_signpost(3,0,0,0,0);

//    }
}

static void
initialize_overhead(iter_t iterations, void* cookie)
{
    int i;
	int procs;
	int* p;

    kdebug_signpost_start(0,0,0,0,0);
	if (iterations) return;
    void* stateptr = (*(void**)cookie);
    struct _state* pStateSrc = (struct _state*)(*(void**)cookie);
    struct _state* pState = (struct _state*)malloc(sizeof(struct _state));
    *pState = *pStateSrc;
    (*(void**)cookie) = pState;
    
	pState->tids = NULL;
	pState->p = (int**)malloc(pState->procs * (sizeof(int*) + 2 * sizeof(int)));
	pState->data = (pState->process_size > 0) ? malloc(pState->process_size) : NULL;
	if (!pState->p || (pState->process_size > 0 && !pState->data)) {
		perror("malloc");
		exit(1);
	}
	p = (int*)&pState->p[pState->procs];
	for (i = 0; i < pState->procs; ++i) {
		pState->p[i] = p;
		p += 2;
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
		close(pState->p[i][0]);
		close(pState->p[i][1]);
	}

	free(pState->p);
	if (pState->data) free(pState->data);
    kdebug_signpost_end(0,0,0,0,0);

}

static void
benchmark_overhead(iter_t iterations, void* cookie)
{
//    struct _state* pState = (struct _state*)cookie;
    struct _state* pState = (struct _state*)(*(void**)cookie);
	int	i = 0;
	int	msg = 1;

	while (iterations-- > 0) {
		if (write(pState->p[i][1], &msg, sizeof(msg)) != sizeof(msg)) {
			/* perror("read/write on pipe"); */
			exit(1);				
		}
		if (read(pState->p[i][0], &msg, sizeof(msg)) != sizeof(msg)) {
			/* perror("read/write on pipe"); */
			exit(1);
		}
		if (++i == pState->procs) {
			i = 0;
		}
		bread(pState->data, pState->process_size);
	}
}

static void
initialize(iter_t iterations, void* cookie)
{
	int procs;

	if (iterations) return;

	initialize_overhead(iterations, cookie);
    struct _state* pState = (struct _state*)(*(void**)cookie);

	pState->tids = (pthread_t*)malloc(pState->procs * sizeof(pthread_t));
	if (pState->tids == NULL)
		exit(1);
	bzero((void*)pState->tids, pState->procs * sizeof(pthread_t));
	procs = create_daemons(pState);
	if (procs < pState->procs) {
		cleanup(0, cookie);
		exit(1);
	}
}

static void
cleanup(iter_t iterations, void* cookie)
{
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
//            kill(pState->tids[i], SIGKILL);
//            waitpid(pState->tids[i], NULL, 0);
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
		if (write(pState->p[0][1], &msg, sizeof(msg)) !=
		    sizeof(msg)) {
			/* perror("read/write on pipe"); */
			exit(1);
		}
		if (read(pState->p[pState->procs-1][0], &msg, sizeof(msg)) != sizeof(msg)) {
			/* perror("read/write on pipe"); */
			exit(1);
		}
		bread(pState->data, pState->process_size);
	}
}


static void
doit(void* args)
{
    state_t *stateptr = (state_t*)args;
    pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
    int i = (int)stateptr->index;
    int rd = stateptr->p[i-1][0];
    int wr = stateptr->p[i][1];
    free(stateptr);
    stateptr = NULL;
	int	msg;
	void*	data = NULL;

	if (process_size) {
		data = malloc(process_size);
		if (!data) {
			perror("malloc");
			exit(3);
		}
		bzero(data, process_size);
	}
	for ( ;; ) {
		if (read(rd, &msg, sizeof(msg)) != sizeof(msg)) {
			/* perror("read/write on pipe"); */
			break;
		}
		if (process_size)
			bread(data, process_size);
		if (write(wr, &msg, sizeof(msg)) != sizeof(msg)) {
			/* perror("read/write on pipe"); */
			break;
		}
	}
//    exit(1);
}


static int
create_daemons(state_t* pStateSrc)
{
	int	i, j;
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

    
//    handle_scheduler(benchmp_childid(), 0, procs-1);
//         for (i = 1; i < procs; ++i) {
//        switch (pids[i] = fork()) {
//            case -1:    /* could not fork, out of processes? */
//            return i;
//
//            case 0:    /* child */
//            handle_scheduler(benchmp_childid(), i, procs-1);
//            for (j = 0; j < procs; ++j) {
//                if (j != i - 1) close(p[j][0]);
//                if (j != i) close(p[j][1]);
//            }
//            doit(p[i-1][0], p[i][1], process_size);
//            /* NOTREACHED */
//
//            default:    /* parent */
//            ;
//            }
//    }

	/*
	 * Go once around the loop to make sure that everyone is ready and
	 * to get the token in the pipeline.
	 */
	if (write(pStateSrc->p[0][1], &msg, sizeof(msg)) != sizeof(msg) ||
	    read(pStateSrc->p[pStateSrc->procs-1][0], &msg, sizeof(msg)) != sizeof(msg)) {
		/* perror("write/read/write on pipe"); */
		exit(1);
	}
	return pStateSrc->procs;
}

static int
create_pipes(int **p, int procs)
{
	int	i;
	/*
	 * Get a bunch of pipes.
	 */
	morefds();
    for (i = 0; i < procs; ++i) {
		if (pipe(p[i]) == -1) {
			return i;
		}
	}
	return procs;
}
