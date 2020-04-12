#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>
#include <fcntl.h>
#include "ex3_q2.h"

pthread_mutex_t stdout_mutex;
pthread_mutex_t buffer_mutex;
pthread_mutex_t create_cnt_mutex;
pthread_mutex_t consume_cnt_mutex;

pthread_mutex_t cond_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond = PTHREAD_COND_INITIALIZER;

sem_t * sems[2];
char* sem_name[2] = { "/empty", "/full" };
int create_cnt = 0;
int consume_cnt = 0;
buffer_t buffer = { .insert_index = 0,.remove_index = 0 };


int main() {
	pthread_t * cons_ids, * prod_ids;
	int ** t_nums = create_tnum_array();
	initialize_semaphores_and_mutex();
	create_cons_prod_threads(&cons_ids, &prod_ids, t_nums);

	wait_for_threads(cons_ids, prod_ids);
	clean_memory(cons_ids, prod_ids, t_nums);
	unlink_semaphores();

	return 0;
}

void unlink_semaphores() {
	int i;
	
	//unlink semaphores
	for (i = 0; i < 2; i++) {
		if (sem_unlink(sem_name[i]) == -1 && errno!=2) {
			printf("error to unlink - value of errno: %d\n ", errno);
			exit(1);
		}
	}
}

void clean_memory(pthread_t * cons_ids, pthread_t * prod_ids, int**t_nums) {
	int i;
	
	free(cons_ids);
	free(prod_ids);

	for (i = 0; i < 2; i++) {
		free(t_nums[i]);
	}

	free(t_nums);
}

void wait_for_threads(pthread_t * cons_ids, pthread_t * prod_ids) {
	int i;
	
	for (i = 0; i < N_PROD; i++) {
		pthread_join(prod_ids[i], NULL);
	}
	printf("all producers terminated\n");

	for (i = 0; i < N_CONS; i++) {
		pthread_join(cons_ids[i], NULL);
	}

	printf("all consumers terminated\n");

}

void initialize_semaphores_and_mutex() {
	unlink_semaphores();

	sems[0] = sem_open(sem_name[0], O_CREAT, S_IRWXU, BUF_SIZE);
	sems[1] = sem_open(sem_name[1], O_CREAT, S_IRWXU, 0);

	if (sems[0] == SEM_FAILED || sems[1] == SEM_FAILED) {
		perror("failed to open semaphore \n"); fflush(stdout);
		exit(1);
	}

	if ((pthread_mutex_init(&stdout_mutex, NULL) != 0) || (pthread_mutex_init(&buffer_mutex, NULL) != 0)) {
		fprintf(stderr, "mutex init has failed \n");
		exit(1);
	}

}

void create_cons_prod_threads(pthread_t ** cons_ids, pthread_t ** prod_ids, int ** t_nums) {
	int i;
	*cons_ids = (pthread_t*)malloc(sizeof(pthread_t) * N_CONS);
	*prod_ids = (pthread_t*)malloc(sizeof(pthread_t) * N_PROD);
	
	for (i = 0; i < N_PROD; i++) {
		pthread_create(*prod_ids + i, NULL, exec_prod, (void*)(t_nums[0]+i));
	}

	printf("main thread created all producer threads\n"); fflush(stdout);

	for (i = 0; i < N_CONS; i++) {
		pthread_create(*cons_ids + i , NULL, exec_cons, (void*)(t_nums[1] + i));
	}

	printf("main thread created all consumer threads\n"); fflush(stdout);

	//give time to last thread get lock
	sleep(1);

	pthread_cond_broadcast(&cond);
}

int ** create_tnum_array() {
	int ** res = (int**)malloc(2 * sizeof(int*));
	int i,j;
	int arr_sizes[2] = { N_PROD, N_CONS };

	for (i = 0; i < 2; i++) {
		res[i] = (int*)malloc(sizeof(int)*arr_sizes[i]);

		for (j = 0; j < arr_sizes[i]; j++) {
			res[i][j] = j + 1;
		}
	}

	return res;
}

void* exec_cons(void*ptr) {
	int t_num = *((int*)ptr);
	pthread_mutex_lock(&cond_mutex);

	pthread_cond_wait(&cond, &cond_mutex);
	pthread_mutex_unlock(&cond_mutex); // unlocking for all other threads

	consumer(t_num);

	return NULL;
}

void* exec_prod(void*ptr) {
	int t_num = *((int*)ptr);

	pthread_mutex_lock(&cond_mutex);

	pthread_cond_wait(&cond, &cond_mutex);
	pthread_mutex_unlock(&cond_mutex); // unlocking for all other threads

	producer(t_num);

	return NULL;
}

void consumer(int thread_number) {
	lluint prod, f1,f2;
	
	while (1) {
		pthread_mutex_lock(&consume_cnt_mutex);
		consume_cnt++;
		
		//check if need to create more
		if (consume_cnt > TOTAL_MSG) {
			pthread_mutex_unlock(&consume_cnt_mutex);
			pthread_mutex_lock(&stdout_mutex);
			write_consumer_is_done(thread_number);
			pthread_mutex_unlock(&stdout_mutex);

			break;
		}

		pthread_mutex_unlock(&consume_cnt_mutex);
		
		sem_wait(sems[1]);					//waiting to full cells
		pthread_mutex_lock(&buffer_mutex);  //waiting to get access ti buffer
		pthread_mutex_lock(&stdout_mutex);  //waiting to get access to stdout

		remove_from_buf(&prod);
		write_remove_from_buf_msg(thread_number, &prod);
		pthread_mutex_unlock(&stdout_mutex);//unlock for other while this find 2 factors
		

		find_two_factors(prod, &f1, &f2);
		
		pthread_mutex_lock(&stdout_mutex);  //waiting to get access to stdout

		printf("consumer #%d", thread_number);
		write_product("", f1, f2, prod);

		pthread_mutex_unlock(&stdout_mutex);
		pthread_mutex_unlock(&buffer_mutex);

		sem_post(sems[0]);
	}
}

void remove_from_buf(lluint* prod) {
	*prod = buffer.products[buffer.remove_index];
	buffer.remove_index = (buffer.remove_index + 1) % BUF_SIZE;
}

void producer(int thread_number) {
	lluint* item;

	while (1) {
		pthread_mutex_lock(&create_cnt_mutex);
		create_cnt++;
		
		//check if need to create more
		if (create_cnt > TOTAL_MSG) {
			pthread_mutex_unlock(&create_cnt_mutex);

			pthread_mutex_lock(&stdout_mutex);
			write_producer_is_done(thread_number);
			pthread_mutex_unlock(&stdout_mutex);

			free(item);

			break;
		}

		pthread_mutex_unlock(&create_cnt_mutex);

		item = produce_item();
		sem_wait(sems[0]); //when there is empty cell
		
		pthread_mutex_lock(&buffer_mutex);	//waiting to get access to buffer
		pthread_mutex_lock(&stdout_mutex);	//waiting to get acess to atdout (print add to buf message)

		write_add_to_buf_msg(thread_number, item[0], item[1], (item[0] * item[1]));
		add_to_buf((item[0]*item[1]));
		

		pthread_mutex_unlock(&stdout_mutex);
		pthread_mutex_unlock(&buffer_mutex);
		sem_post(sems[1]);
	}
}

lluint * produce_item() {
	lluint * primes = (lluint*)malloc(sizeof(lluint)*2 );
	int i;

	for (i = 0; i < 2; i++){
		while (1) {
			primes[i] = get_random_in_range();
			if (is_prime(primes[i])) {
				break;
			}
		}
	}

	return primes;
}

void add_to_buf(lluint prod) {
	buffer.products[buffer.insert_index] = prod;
	buffer.insert_index = (buffer.insert_index + 1) % BUF_SIZE;
}
