/*
** scheduler module for executing lua processes
** See Copyright Notice in luaproc.h
*/

#ifndef _LUA_LUAPROC_SCHED_H_
#define _LUA_LUAPROC_SCHED_H_

#include "luaproc.h"

/****************************************
 * scheduler functions return constants *
 ***************************************/

/* scheduler function return constants */
#define	LUAPROC_SCHED_OK                 0
#define LUAPROC_SCHED_PTHREAD_ERROR     -1

/*************************************
 * default number of initial workers *
 ************************************/

/* scheduler default number of worker threads */
#define LUAPROC_SCHED_DEFAULT_WORKER_THREADS 1

/***********************
 * function prototypes *
 **********************/

/* initialize scheduler */
int sched_init( void );
/* join workers */
void sched_join_workers( void );
/* wait until there are no more active lua processes */
void sched_wait( void );
/* move process to ready queue (ie, schedule process) */
void sched_queue_proc( luaproc *lp );
/* increase active luaproc count */
void sched_inc_lpcount( void );
/* set number of active workers (creates and destroys accordingly) */
int sched_set_numworkers( int numworkers );
/* return the number of active workers */
int sched_get_numworkers( void );

#endif
