/*
** scheduler module for executing lua processes
** See Copyright Notice in luaproc.h
*/

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>

#include "lpsched.h"
#include "luaproc.h"

#define FALSE 0
#define TRUE  !FALSE
#define LUAPROC_SCHED_WORKERS_TABLE "workertb"

#if (LUA_VERSION_NUM >= 502)
#define luaproc_resume( L, from, nargs ) lua_resume( L, from, nargs )
#else
#define luaproc_resume( L, from, nargs ) lua_resume( L, nargs )
#endif

/********************
 * global variables *
 *******************/

/* ready process list */
list ready_lp_list;

/* ready process queue access mutex */
pthread_mutex_t mutex_sched = PTHREAD_MUTEX_INITIALIZER;

/* active luaproc count access mutex */
pthread_mutex_t mutex_lp_count = PTHREAD_MUTEX_INITIALIZER;

/* wake worker up conditional variable */
pthread_cond_t cond_wakeup_worker = PTHREAD_COND_INITIALIZER;

/* no active luaproc conditional variable */
pthread_cond_t cond_no_active_lp = PTHREAD_COND_INITIALIZER;

/* lua_State used to store workers hash table */
static lua_State *workerls = NULL;

int lpcount = 0;         /* number of active luaprocs */
int workerscount = 0;    /* number of active workers */
int destroyworkers = 0;  /* number of workers to destroy */

/***********************
 * register prototypes *
 ***********************/

static void sched_dec_lpcount( void );

/*******************************
 * worker thread main function *
 *******************************/

/* worker thread main function */
void *workermain( void *args ) {

  luaproc *lp;
  int procstat;

  /* main worker loop */
  while ( TRUE ) {
    /*
      wait until instructed to wake up (because there's work to do
      or because workers must be destroyed)
    */
    pthread_mutex_lock( &mutex_sched );
    while (( list_count( &ready_lp_list ) == 0 ) && ( destroyworkers <= 0 )) {
      pthread_cond_wait( &cond_wakeup_worker, &mutex_sched );
    }

    if ( destroyworkers > 0 ) {  /* check whether workers should be destroyed */
      
      destroyworkers--; /* decrease workers to be destroyed count */
      workerscount--; /* decrease active workers count */

      /* remove worker from workers table */
      lua_getglobal( workerls, LUAPROC_SCHED_WORKERS_TABLE );
      lua_pushlightuserdata( workerls, (void *)pthread_self( ));
      lua_pushnil( workerls );
      lua_rawset( workerls, -3 );
      lua_pop( workerls, 1 );

      pthread_cond_signal( &cond_wakeup_worker );  /* wake other workers up */
      pthread_mutex_unlock( &mutex_sched );
      pthread_exit( NULL );  /* destroy itself */
    }

    /* remove lua process from the ready queue */
    lp = list_remove( &ready_lp_list );
    pthread_mutex_unlock( &mutex_sched );

    /* execute the lua code specified in the lua process struct */
    procstat = luaproc_resume( luaproc_get_state( lp ), NULL,
                               luaproc_get_numargs( lp ));
    /* reset the process argument count */
    luaproc_set_numargs( lp, 0 );

    /* has the lua process sucessfully finished its execution? */
    if ( procstat == 0 ) {
      luaproc_set_status( lp, LUAPROC_STATUS_FINISHED );  
      luaproc_recycle_insert( lp );  /* try to recycle finished lua process */
      sched_dec_lpcount();  /* decrease active lua process count */
    }

    /* has the lua process yielded? */
    else if ( procstat == LUA_YIELD ) {

      /* yield attempting to send a message */
      if ( luaproc_get_status( lp ) == LUAPROC_STATUS_BLOCKED_SEND ) {
        luaproc_queue_sender( lp );  /* queue lua process on channel */
        /* unlock channel */
        luaproc_unlock_channel( luaproc_get_channel( lp ));
      }

      /* yield attempting to receive a message */
      else if ( luaproc_get_status( lp ) == LUAPROC_STATUS_BLOCKED_RECV ) {
        luaproc_queue_receiver( lp );  /* queue lua process on channel */
        /* unlock channel */
        luaproc_unlock_channel( luaproc_get_channel( lp ));
      }

      /* yield on explicit coroutine.yield call */
      else { 
        /* re-insert the job at the end of the ready process queue */
        pthread_mutex_lock( &mutex_sched );
        list_insert( &ready_lp_list, lp );
        pthread_mutex_unlock( &mutex_sched );
      }
    }

    /* or was there an error executing the lua process? */
    else {
      /* print error message */
      fprintf( stderr, "close lua_State (error: %s)\n",
               luaL_checkstring( luaproc_get_state( lp ), -1 ));
      lua_close( luaproc_get_state( lp ));  /* close lua state */
      sched_dec_lpcount();  /* decrease active lua process count */
    }
  }    
}

/***********************
 * auxiliary functions *
 **********************/

/* decrease active lua process count */
static void sched_dec_lpcount( void ) {
  pthread_mutex_lock( &mutex_lp_count );
  lpcount--;
  /* if count reaches zero, signal there are no more active processes */
  if ( lpcount == 0 ) {
    pthread_cond_signal( &cond_no_active_lp );
  }
  pthread_mutex_unlock( &mutex_lp_count );
}

/**********************
 * exported functions *
 **********************/

/* increase active lua process count */
void sched_inc_lpcount( void ) {
  pthread_mutex_lock( &mutex_lp_count );
  lpcount++;
  pthread_mutex_unlock( &mutex_lp_count );
}

/* local scheduler initialization */
int sched_init( void ) {

  int i;
  pthread_t worker;

  /* initialize ready process list */
  list_init( &ready_lp_list );

  /* initialize workers table and lua_State used to store it */
  workerls = luaL_newstate();
  lua_newtable( workerls );
  lua_setglobal( workerls, LUAPROC_SCHED_WORKERS_TABLE );

  /* get ready to access worker threads table */
  lua_getglobal( workerls, LUAPROC_SCHED_WORKERS_TABLE );

  /* create default number of initial worker threads */
  for ( i = 0; i < LUAPROC_SCHED_DEFAULT_WORKER_THREADS; i++ ) {

    if ( pthread_create( &worker, NULL, workermain, NULL ) != 0 ) {
      lua_pop( workerls, 1 ); /* pop workers table from stack */
      return LUAPROC_SCHED_PTHREAD_ERROR;
    }

    /* store worker thread id in a table */
    lua_pushlightuserdata( workerls, (void *)worker );
    lua_pushboolean( workerls, TRUE );
    lua_rawset( workerls, -3 );

    workerscount++; /* increase active workers count */
  }

  lua_pop( workerls, 1 ); /* pop workers table from stack */

  return LUAPROC_SCHED_OK;
}

/* set number of active workers */
int sched_set_numworkers( int numworkers ) {

  int i, delta;
  pthread_t worker;

  pthread_mutex_lock( &mutex_sched );

  /* calculate delta between existing workers and set number of workers */
  delta = numworkers - workerscount;

  /* create additional workers */
  if ( numworkers > workerscount ) {

    /* get ready to access worker threads table */
    lua_getglobal( workerls, LUAPROC_SCHED_WORKERS_TABLE );

    /* create additional workers */
    for ( i = 0; i < delta; i++ ) {

      if ( pthread_create( &worker, NULL, workermain, NULL ) != 0 ) {
        pthread_mutex_unlock( &mutex_sched );
        lua_pop( workerls, 1 ); /* pop workers table from stack */
        return LUAPROC_SCHED_PTHREAD_ERROR;
      }

      /* store worker thread id in a table */
      lua_pushlightuserdata( workerls, (void *)worker );
      lua_pushboolean( workerls, TRUE );
      lua_rawset( workerls, -3 );

      workerscount++; /* increase active workers count */
    }

    lua_pop( workerls, 1 ); /* pop workers table from stack */
  }
  /* destroy existing workers */
  else if ( numworkers < workerscount ) {
    destroyworkers = destroyworkers + numworkers;
  }

  pthread_mutex_unlock( &mutex_sched );

  return LUAPROC_SCHED_OK;
}

/* return the number of active workers */
int sched_get_numworkers( void ) {

  int numworkers;

  pthread_mutex_lock( &mutex_sched );
  numworkers = workerscount;
  pthread_mutex_unlock( &mutex_sched );

  return numworkers;
}

/* insert lua process in ready queue */
void sched_queue_proc( luaproc *lp ) {
  pthread_mutex_lock( &mutex_sched );
  list_insert( &ready_lp_list, lp );  /* add process to ready queue */
  /* set process status ready */
  luaproc_set_status( lp, LUAPROC_STATUS_READY );
  pthread_cond_signal( &cond_wakeup_worker );  /* wake worker up */
  pthread_mutex_unlock( &mutex_sched );
}

/* join worker threads (called when Lua exits). not joining workers causes a
   race condition since lua_close unregisters dynamic libs with dlclose and
   thus libpthreads can be unloaded while there are workers that are still 
   alive. */
void sched_join_workers( void ) {

  lua_State *L = luaL_newstate();
  const char *wtb = "workerstbcopy";

  /* wait for all running lua processes to finish */
  sched_wait();

  /* initialize new state and create table to copy worker ids */
  lua_newtable( L );
  lua_setglobal( L, wtb );
  lua_getglobal( L, wtb );

  pthread_mutex_lock( &mutex_sched );

  /* determine remaining active worker threads and copy their ids */
  lua_getglobal( workerls, LUAPROC_SCHED_WORKERS_TABLE );
  lua_pushnil( workerls );
  while ( lua_next( workerls, -2 ) != 0 ) {
    lua_pushlightuserdata( L, lua_touserdata( workerls, -2 ));
    lua_pushboolean( L, TRUE );
    lua_rawset( L, -3 );
    /* pop value, leave key for next iteration */
    lua_pop( workerls, 1 );
  }

  /* pop workers copy table name from stack */
  lua_pop( L, 1 );

  /* set all workers to be destroyed */
  destroyworkers = workerscount;

  /* wake workers up */
  pthread_cond_signal( &cond_wakeup_worker );
  pthread_mutex_unlock( &mutex_sched );

  /* join with worker threads (read ids from local table copy ) */
  lua_getglobal( L, wtb );
  lua_pushnil( L );
  while ( lua_next( L, -2 ) != 0 ) {
    pthread_join(( pthread_t )lua_touserdata( L, -2 ), NULL );
    /* pop value, leave key for next iteration */
    lua_pop( L, 1 );
  }
  lua_pop( L, 1 );

  lua_close( workerls );
  lua_close( L );
}

/* wait until there are no more active lua processes and active workers. */
void sched_wait( void ) {

  /* wait until there are not more active lua processes */
  pthread_mutex_lock( &mutex_lp_count );
  if( lpcount != 0 ) {
    pthread_cond_wait( &cond_no_active_lp, &mutex_lp_count );
  }
  pthread_mutex_unlock( &mutex_lp_count );

}
