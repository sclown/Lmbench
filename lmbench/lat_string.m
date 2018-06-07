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

#include "bench.h"
#include <Foundation/Foundation.h>

static void initialize(iter_t iterations, void *cookie);
static void cleanup(iter_t iterations, void *cookie);
static void doit(iter_t iterations, void *cookie);

typedef struct _state {
    CFMutableArrayRef array;
    const char* path;
} state_t;

void
lat_string(const char* documents)
{
	state_t state;
    state.path = documents;
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
	micro("Add object", get_n());
}

static void
initialize(iter_t iterations, void* cookie)
{
    if (iterations) {
        state_t * state = (state_t *)(*(void**)cookie);
        NSMutableArray *array = (__bridge NSMutableArray*)state->array;
        char buf[10048];
        char path[2048];
        strcpy(path, state->path);
        strcat(path, "/Build.log");
        int fd = open(path, O_RDONLY);
        while (iterations-- > 0) {
            
            if (read(fd, buf, 1024) <= 0) {
                break;
            }
            buf[1024] = 0;
            NSString *str = [NSString stringWithCString:buf encoding:NSISOLatin1StringEncoding];
            [array addObject:str];
            if(array.count > 1000) {
                break;
            }
        }
        close(fd);
        if(iterations != (u_long)-1) {
            unsigned long readSize = array.count;
            while (iterations-- > 0) {
                [array addObject:array[iterations%readSize]];
            }

        }
       return;
    };
    
    state_t * state = (state_t *)(*(void**)cookie);
    NSMutableArray *array = [[NSMutableArray alloc] init];
    state->array =  (__bridge_retained CFMutableArrayRef)array;

}

static void
cleanup(iter_t iterations, void* cookie)
{

    if (iterations){
        state_t * state = (state_t *)(*(void**)cookie);
        NSMutableArray *array = (__bridge NSMutableArray*)state->array;
        [array removeAllObjects];
        return;
    }

    state_t * state = (state_t *)(*(void**)cookie);
    NSMutableArray *array = (__bridge_transfer NSMutableArray*)state->array;
    array = NULL;
}

static void
doit(register iter_t iterations, void *cookie)
{
    state_t * state = (state_t *)(*(void**)cookie);
	while (iterations-- > 0) {
        NSMutableArray *array = (__bridge NSMutableArray*)state->array;
        NSString *data = array[iterations];
        NSRange searchRange = NSMakeRange(0,data.length);
        NSRange foundRange;
        int count = 0;
        while (searchRange.location < data.length) {
            searchRange.length = data.length-searchRange.location;
            foundRange = [data rangeOfString:@"find" options:0 range:searchRange];
            if (foundRange.location != NSNotFound) {
                ++count;
                searchRange.location = foundRange.location+foundRange.length;
            } else {
                // no more substring to find
                break;
            }
        }
        array[iterations] = @(count);
	}
}
