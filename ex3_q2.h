
#ifndef EX3_Q2_H
#define EX3_Q2_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <unistd.h>
#include <sys/wait.h> 
#include <semaphore.h>
#include <pthread.h>
#include "ex3_q2_given.h"

typedef struct {
	lluint products[BUF_SIZE];	//buffer
	int insert_index;
	int remove_index;
} buffer_t;


void wait_for_threads(pthread_t * cons_ids, pthread_t * prod_ids);
void create_cons_prod_threads(pthread_t ** cons_ids, pthread_t ** prod_ids, int **t_nums);
int ** create_tnum_array();
void* exec_prod(void*ptr);
void* exec_cons(void*ptr);
void initialize_semaphores_and_mutex();
void* exec_cons(void*ptr);
void consumer(int thread_number);
void producer(int thread_num);
lluint * produce_item();
void unlink_semaphores();
void clean_memory(pthread_t * cons_ids, pthread_t * prod_ids, int**t_nums);

#endif
