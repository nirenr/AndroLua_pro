/*
** Auxiliary functions from luaproc API
** See Copyright Notice at the end of this file
*/

#ifndef _LUA_LUAPROC_H_
#define _LUA_LUAPROC_H_

/*************************************
 * execution status of lua processes *
 ************************************/

#define LUAPROC_STATUS_IDLE           0
#define LUAPROC_STATUS_READY          1
#define LUAPROC_STATUS_BLOCKED_SEND   2
#define LUAPROC_STATUS_BLOCKED_RECV   3
#define LUAPROC_STATUS_FINISHED       4

/*******************
 * structure types *
 ******************/

typedef struct stluaproc luaproc; /* lua process */

typedef struct stchannel channel; /* communication channel */

/* linked (fifo) list */
typedef struct stlist {
  luaproc *head;
  luaproc *tail;
  int nodes;
} list;

/***********************
 * function prototypes *
 **********************/

/* unlock access to a channel */
void luaproc_unlock_channel( channel *chan );

/* return a channel where a lua process is blocked at */
channel *luaproc_get_channel( luaproc *lp );

/* queue a lua process that tried to send a message */
void luaproc_queue_sender( luaproc *lp );

/* queue a lua process that tried to receive a message */
void luaproc_queue_receiver( luaproc *lp );

/* add a lua process to the recycle list */
void luaproc_recycle_insert( luaproc *lp );

/* return a lua process' status */
int luaproc_get_status( luaproc *lp );

/* set a lua process' status */
void luaproc_set_status( luaproc *lp, int status );

/* return a lua process' lua state */
lua_State *luaproc_get_state( luaproc *lp );

/* return the number of arguments expected by a lua process */
int luaproc_get_numargs( luaproc *lp );

/* set the number of arguments expected by a lua process */
void luaproc_set_numargs( luaproc *lp, int n );

/* initialize an empty list */
void list_init( list *l );

/* insert a lua process in a list */
void list_insert( list *l, luaproc *lp );

/* remove and return the first lua process in a list */
luaproc* list_remove( list *l );

/* return a list's node count */
int list_count( list *l );

/* }====================================================================== */


/******************************************************************************
* Copyright 2008-2015 Alexandre Skyrme, Noemi Rodriguez, Roberto Ierusalimschy
*
* Permission is hereby granted, free of charge, to any person obtaining
* a copy of this software and associated documentation files (the
* "Software"), to deal in the Software without restriction, including
* without limitation the rights to use, copy, modify, merge, publish,
* distribute, sublicense, and/or sell copies of the Software, and to
* permit persons to whom the Software is furnished to do so, subject to
* the following conditions:
*
* The above copyright notice and this permission notice shall be
* included in all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
******************************************************************************/

#endif
