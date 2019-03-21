/*
 * bw_file_rd.c - time reading & summing of a file
 *
 * Usage: bw_file_rd [-C] [-P <parallelism] [-W <warmup>] [-N <repetitions>] size file
 *
 * The intent is that the file is in memory.
 * Disk benchmarking is done with lmdd.
 *
 * Copyright (c) 1994 Larry McVoy.  Distributed under the FSF GPL with
 * additional restriction that results may published only if
 * (1) the benchmark is unmodified, and
 * (2) the version in the sccsid below is included in the report.
 * Support for this development by Sun Microsystems is gratefully acknowledged.
 */
static char	*id = "$Id$\n";

#include "bench.h"
#include <pthread.h>

#define	CHK(x)		if ((int)(x) == -1) { perror(#x); exit(1); }
#ifndef	MIN
#define	MIN(a, b)	((a) < (b) ? (a) : (b))
#endif

#define	TYPE	int
#define	MINSZ	(sizeof(TYPE) * 128)

#define KDEBUG
#include "kdebug.h"

void	*buf;		/* do the I/O here */
size_t	xfersize;	/* do it in units of this */
size_t	count;		/* bytes to move (can't be modified) */

typedef struct _state {
	char filename[256];
	int fd;
	int clone;
} state_t;

static void doit(int fd)
{
	size_t	size, chunk;

	size = count;
    void* ptr = buf;
	chunk = xfersize;
	while (size > 0) {
		if (size < chunk) chunk = size;
        KDSTART(4)
		if (read(fd, ptr, MIN(size, chunk)) <= 0) {
			break;
		}
        KDEND(4)
		size -= chunk;
        ptr += chunk;
	}
    KDSTART(5)
    bread(buf, size);
    KDEND(5)
}

static void
initialize(iter_t iterations, void* cookie)
{

	if (iterations) return;
    state_t    *stateSrc = (state_t *)(*(void**)cookie);
    state_t *state = (state_t*)malloc(sizeof(state_t));
    *state = *stateSrc;
    *(void**)cookie = state;

	state->fd = -1;
	if (state->clone) {
		char buf[128];
		char* s;

		/* copy original file into a process-specific one */
		sprintf(buf, "%d", (int)pthread_self());
		s = (char*)malloc(strlen(state->filename) + strlen(buf) + 1);
		sprintf(s, "%s%d", state->filename, (int)getpid());
		if (cp(state->filename, s, S_IREAD|S_IWRITE) < 0) {
			perror("creating private tempfile");
			unlink(s);
			exit(1);
		}
		strcpy(state->filename, s);
	}
}

static void
init_open(iter_t iterations, void * cookie)
{
	int	ofd;

	if (iterations) return;

	initialize(0, cookie);
    state_t    *state = (state_t *)(*(void**)cookie);
	CHK(ofd = open(state->filename, O_RDONLY));
	state->fd = ofd;
}

static void
time_with_open(iter_t iterations, void * cookie)
{
    state_t    *state = (state_t *)(*(void**)cookie);
	char	*filename = state->filename;
	int	fd;

	while (iterations-- > 0) {
        KDSTART(2);
		fd = open(filename, O_RDONLY);
		doit(fd);
		close(fd);
        KDEND(2);
	}
}

static void
time_io_only(iter_t iterations,void * cookie)
{
    state_t    *state = (state_t *)(*(void**)cookie);
	int fd = state->fd;

	while (iterations-- > 0) {
		lseek(fd, 0, SEEK_SET);
		doit(fd);
	}
}

static void
cleanup(iter_t iterations, void * cookie)
{
    state_t    *state = (state_t *)(*(void**)cookie);

	if (iterations) return;

	if (state->fd >= 0) close(state->fd);
	if (state->clone) unlink(state->filename);
}

void
bw_file_rd_size(const char* documents, int size, int chuncked)
{
	state_t state;
	int	parallel = 1;
	int	warmup = 0;
	int	repetitions = -1;
	state.clone = 0;


    strcpy(state.filename, documents);
	strcat(state.filename,"/Lmbench");
	count = bytes("1K");
    count = size;
    if (count < XFERSIZE) {
        xfersize = count;
    } else {
        xfersize = XFERSIZE;
    }
    if(!chuncked) {
        xfersize = count;
    }
    
	buf = (void *)valloc(size);
	bzero(buf, size);

    KDSTART(1);
//    {//open2close
//        benchmp(initialize, time_with_open, cleanup,
//                0, parallel, warmup, repetitions, &state);
//    }
    {//io_only
        benchmp(init_open, time_io_only, cleanup,
                0, parallel, warmup, repetitions, &state);
    }
	bandwidth(count, get_n() * parallel, 1);
    KDEND(1);
}

void
bw_file_rd(const char* documents) {
    bw_file_rd_size(documents, 1, FALSE);
    bw_file_rd_size(documents, 100, FALSE);
    bw_file_rd_size(documents, MINSZ, FALSE);
    bw_file_rd_size(documents, 1024, FALSE);
    bw_file_rd_size(documents, 5*1024, FALSE);
    bw_file_rd_size(documents, 10*1024, FALSE);
    bw_file_rd_size(documents, 16*1024, FALSE);
    bw_file_rd_size(documents, 64*1024, FALSE);
    bw_file_rd_size(documents, 512*1024, FALSE);
    bw_file_rd_size(documents, 1024*1024, FALSE);
    bw_file_rd_size(documents, 3*1024*1024, FALSE);
    bw_file_rd_size(documents, 5*1024*1024, FALSE);
    bw_file_rd_size(documents, 10*1024*1024, FALSE);


    bw_file_rd_size(documents, 1, TRUE);
    bw_file_rd_size(documents, 100, TRUE);
    bw_file_rd_size(documents, MINSZ, TRUE);
    bw_file_rd_size(documents, 1024, TRUE);
    bw_file_rd_size(documents, 5*1024, TRUE);
    bw_file_rd_size(documents, 10*1024, TRUE);
    bw_file_rd_size(documents, 16*1024, TRUE);
    bw_file_rd_size(documents, 64*1024, TRUE);
    bw_file_rd_size(documents, 512*1024, TRUE);
    bw_file_rd_size(documents, 1024*1024, TRUE);
    bw_file_rd_size(documents, 3*1024*1024, TRUE);
    bw_file_rd_size(documents, 5*1024*1024, TRUE);
    bw_file_rd_size(documents, 10*1024*1024, TRUE);
}

