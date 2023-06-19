#ifndef _JTIMER_H_
#define _JTIMER_H_

//#define _POSIX_C_SOURCE 199309L
/*#ifdef _GNU_SOURCE
# undef  _XOPEN_SOURCE
# define _XOPEN_SOURCE 600
# undef  _XOPEN_SOURCE_EXTENDED
# define _XOPEN_SOURCE_EXTENDED 1
# undef  _LARGEFILE64_SOURCE
# define _LARGEFILE64_SOURCE 1
# undef  _BSD_SOURCE
# define _BSD_SOURCE 1
# undef  _SVID_SOURCE
# define _SVID_SOURCE 1
# undef  _ISOC99_SOURCE
# define _ISOC99_SOURCE 1
# undef  _POSIX_SOURCE
# define _POSIX_SOURCE 1
# undef  _POSIX_C_SOURCE
# define _POSIX_C_SOURCE 200112L
# undef  _ATFILE_SOURCE
# define _ATFILE_SOURCE 1
#endif*/
#include <stdio.h>
#include <signal.h>
#include <time.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include "utildbg.h" 
#define CLOCKID CLOCK_REALTIME
//#define CLOCKID CLOCK_MONOTONIC


timer_t jset_timer(timer_t start_sec, long start_nsec, timer_t interval_sec, long interval_nsec, void* func, int sival);


#endif
