/*
 * CS533 Course Project
 * Thread Queue ADT
 * queue.h
 *
 * Feel free to modify this file. Please thoroughly comment on
 * any changes you make.
 */

 /*Modified by Lai Xu Oct.20.2014 */
#ifndef __QUEUE_H__
#define __QUEUE_H__

#include "scheduler.h"

struct queue_node {
  struct thread_t * t;
  struct queue_node * next;  
};

struct queue {
  struct queue_node * head;
  struct queue_node * tail;
};


/* enqueue a thread */
int thread_enqueue(struct queue * q, struct thread_t * t);

/* dequeue a thread */
struct thread_t * thread_dequeue(struct queue * q);

/* is the queue empty? */
int is_empty(struct queue * q);

/*
   Modified by Lai Xu Oct.20.2014 
   print the content of the queue 
   for debug purpose
*/
void print_queue(struct queue* q);

#endif //__QUEUE_H__
