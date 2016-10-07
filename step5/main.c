/*************************************************
 *
 *  CS533 Project: User level thread
 *  File: main.c
 *  Author:
 *        Lai Xu  929272440 (xulai@pdx.edu)
 *        Bo Chen 904039384 (chenbo@pdx.edu)
 *  Date: DEC.09.2014
 *  Desc: A benchmark for testing the scalability
 *        of our thread library
 *
*************************************************/


#include <stdio.h>
#include <math.h>
#include <time.h>
#include <string.h>
#include "scheduler.h"
//#include "mutex.h"


typedef struct bigdata_args {
  size_t element_size;
  void *arr;
  int  begin;
  int  end;
  int* flag;
}bigdata_args;


/* The computation task performed by each thread */
void task(void* arg){
  if (arg == NULL){
    return;
  }

  bigdata_args* task_arg = (bigdata_args*)arg;

  int i = task_arg -> begin;
  double *arr = (double*)task_arg->arr;

  while (i <= task_arg->end){

    arr[i] = sin((float)i);

    int j = 10;
    while(j--){}

    i++;
  }

  *(task_arg->flag) = 1;

}

/* create a large data array and divide into subtasks,
   assigning to the threads*/
void benchmark(int total_task, int task_per_thread)
{

  int begin = 0;
  int end = 0;

  double* tasks = (double*)malloc(total_task* sizeof(double));
  bigdata_args* arg = NULL;

  int thread_num = ceil(total_task/task_per_thread);

  int* finish_flags = malloc(sizeof(int)*thread_num);
  memset(finish_flags, 0, sizeof(int)*thread_num);

  int thread_counter = 0;

  while(1){
    if (begin >= total_task){
      break;
    }

    end = begin + task_per_thread - 1;

    if (end >= total_task){
      end = total_task - 1;
    }


    arg = (bigdata_args*)malloc(sizeof(bigdata_args));
    arg->arr = tasks;
    arg->begin = begin;
    arg->end = end;
    arg->flag = finish_flags + thread_counter;

    thread_fork(task, arg);
    thread_counter++;

    begin = end + 1;

  }

  int finished, i;

  do{
    finished = 1;

    for (i = 0; i < thread_num; i++){
      finished = finished && finish_flags[i];
    }

  }while(!finished);

}


int main(int argc, char *argv[])
{

    if (argc < 2){
        printf("Usage: step5 [number of kernel threads]\n");
        return -1;
    }

    int num_kernel = atoi(argv[1]);
    scheduler_begin(num_kernel);

    benchmark(200000000, 10000000);

    scheduler_end();
    return 0;
}
