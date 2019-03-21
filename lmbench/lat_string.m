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

#define KDEBUG
#include "kdebug.h"

static void initialize(iter_t iterations, void *cookie);
static void cleanup(iter_t iterations, void *cookie);
static void doit(iter_t iterations, void *cookie);

static int arraysize = 100;

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

    for(arraysize=100000; arraysize <= 1000000; arraysize += 100000) {
        benchmp(initialize, doit, cleanup, SHORT, parallel,
                warmup, repetitions, &state);
        micro("Add object", get_n());
    }
}

static void
initialize(iter_t iterations, void* cookie)
{
    if (iterations) {
        state_t * state = (state_t *)(*(void**)cookie);
        NSMutableArray *array = (__bridge NSMutableArray*)state->array;
        char* buf = malloc(arraysize+1);
        char path[2048];
        strcpy(path, state->path);
        strcat(path, "/Build.log");
        int fd = open(path, O_RDONLY);
        while (iterations-- > 0) {
        
//            for (int i=0; i<100; ++i) {
                if (read(fd, buf, arraysize) <= 0) {
                    break;
                }
                buf[arraysize] = 0;
                NSString *str = [NSString stringWithCString:buf encoding:NSISOLatin1StringEncoding];
                [array addObject:str];
                if(array.count > 100) {
                    break;
                }
//            }
        }
        close(fd);
        free(buf);
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
        KDSTART(2)
        NSMutableArray *array = (__bridge NSMutableArray*)state->array;
        NSString *data = array[iterations%array.count];
        NSRange searchRange = NSMakeRange(0,data.length);
        NSRange foundRange;
        int count = 0;
        KDSTART(3)
        while (searchRange.location < data.length) {
            searchRange.length = data.length-searchRange.location;
            foundRange = [data rangeOfString:@"connection" options:0 range:searchRange];
            if (foundRange.location != NSNotFound) {
                ++count;
                searchRange.location = foundRange.location+foundRange.length;
            } else {
                // no more substring to find
                break;
            }
        }
        KDEND(3)
        KDEND(2)
//        array[iterations%arraysize] = @(count);
	}
}
