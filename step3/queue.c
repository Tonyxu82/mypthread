/*
 * CS533 Course Project
 * Thread Queue ADT
 * queue.c
 *
 * Feel free to modify this file. Please thoroughly comment on
 * any changes you make.
 */

#include <stdlib.h>
#include <stdio.h>
#include "queue.h"

/*Modified by Lai Xu Oct.20.2014 for adding return value*/
int thread_enqueue(struct queue* q, struct thread_t* t) {

  /*Modified by Lai Xu Oct.20.2014 for judging the pointer q*/
  if (NULL == q || NULL == t)
  {
    return -1;
  }


  struct queue_node * temp = malloc(sizeof(struct queue_node));
  /*Modified by Lai Xu Oct.20.2014 */
  if (NULL == temp)
  {
    return -1;
  }

  temp->t = t;
  temp->next = NULL;

  if(!q->head) {
    q->head = q->tail = temp;
    return 0;
  }

  q->tail->next = temp;
  q->tail = temp;

}

struct thread_t * thread_dequeue(struct queue * q) {

  if(!q->head) {
    return NULL;
  }

  struct thread_t * t = q->head->t;
  struct queue_node * temp = q->head;
  q->head = q->head->next;
  free(temp);

  if(!q->head) {
    q->tail = NULL;
  }

  return t;

}

int is_empty(struct queue * q) {
  return !q->head;
}

/*Modified by Lai Xu Oct.20.2014 */
void print_queue(struct queue* q)
{
  if(NULL == q)
  {
    return;
  }

  struct queue_node* temp = q->head;

  printf("Q:");
  while(temp)
  {
      if(temp->t)
      {
        printf("%lu->", temp->t->i_tid);

      }
      else
      {
        printf("[]->");
      }
      temp = temp->next;


  }
  printf("NUL\n");



}
