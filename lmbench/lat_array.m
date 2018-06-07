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
    pthread_t writer_tid;
    CFMutableArrayRef array;
} state_t;

void
lat_array()
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
	micro("Add object", get_n());
}

static void
initialize(iter_t iterations, void* cookie)
{
    if (iterations) {
        state_t * state = (state_t *)(*(void**)cookie);
        NSMutableArray *array = (__bridge NSMutableArray*)state->array;
        for(int i=0; i<iterations;++i) {
            NSMutableArray *sub = [NSMutableArray new];
            for(int j=0; j<9000; ++j) {
                [sub addObject:@(rand())];
            }
            [array addObject:sub];
            
        }
        return;
    };
    
    state_t * state = (state_t *)(*(void**)cookie);
    NSMutableArray *array = [[NSMutableArray alloc] init];
    [array addObject:[NSMutableArray new]];
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
        NSArray *childArray = array[iterations];
        array[iterations] = [childArray sortedArrayUsingComparator:^NSComparisonResult(NSNumber*  _Nonnull obj1, NSNumber*  _Nonnull obj2) {
            if(obj1.integerValue < obj2.integerValue) {
                return NSOrderedAscending;
            }
            else if(obj1.integerValue > obj2.integerValue) {
                return NSOrderedDescending;
            }
            return NSOrderedSame;
        }];
	}
}
