#ifndef _KDEBUG_H
#define _KDEBUG_H

#include <sys/kdebug_signpost.h>

#ifdef KDEBUG

#define KDSIGN(x) kdebug_signpost(x,0,0,0,x);
#define KDSTART(x) kdebug_signpost_start(x,0,0,0,x);
#define KDEND(x) kdebug_signpost_end(x,0,0,0,x);

#else

#define KDSIGN(x)
#define KDSTART(x)
#define KDEND(x)

#endif

#endif /* _KDEBUG_H */
