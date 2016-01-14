#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <math.h>
#include <pthread.h>

#include "structures.h"
#include "global_functions.h"
#include "worker_functions.h"

int main(int argc, char** argv){
	uint32_t i;
	pthread_t* worker_threads;
	
	//arguments check
	check_arguments(argc, argv);
	printf("Arguments OK.\n");
	printf("\n");
	
	//loading FAT file
	load_file(input_file_path);
	printf("File loaded into memory.\n");
	printf("\n");
	
	//printing of loaded structures for control
	print_structures();
	printf("End of contents.\n");
	printf("\n");
	
	//allocate memory for threads
	worker_threads = (pthread_t*) malloc(sizeof(pthread_t) * threads_cnt);
	
	//preparation of the file size check task
	processed_files = 0;
	bad_file_size_sum = 0;
	printf("Starting file size check task...\n");
	printf("\n");
	
	//creating threads for the file size check task
	for(i = 0; i < threads_cnt; i++) {
		pthread_create(worker_threads[i], NULL, do_work, CHECK_FILE_SIZE_JOB);
	}
	
	//joining the threads
	for(i = 0; i < threads_cnt; i++) {
		pthread_join(worker_threads[i]);
	}
	
	//if some files had bad size, it won't continue with another task
	if(bad_file_size_sum > 0) {
		printf("There have been %d files with bad size in total.\n", bad_file_size_sum);
		printf("\n");
	}
	else {
		printf("All files has correct size. Continuing with defragmentation task...\n");
		printf("\n");
		
		//initialize mutex array for locking clusters
		for(i = 0; i < CLUSTER_MUTEX_CNT; i++) {
			cluster_mutex_array[i] = PTHREAD_MUTEX_INITIALIZER;
		}
		
		//preparation of the defragmentation task
		processed_files = 0;
		prepare_fat_tables_for_defrag();
		
		//creating threads for the defragmentation task
		for(i = 0; i < threads_cnt; i++) {
			pthread_create(worker_threads[i], NULL, do_work, DEFRAGMENATATION_JOB);
		}
		
		//joining the threads
		for(i = 0; i < threads_cnt; i++) {
			pthread_join(worker_threads[i]);
		}
	}
	
	//saving altered FAT file
	if((save_file(output_file_path) == 0)) {
		printf("File was successfully saved.\n");
	}
	else {
		printf("File couldn't be saved.\n");
	}
	printf("\n");
	
	//de-allocation of dynamically used memory
	free(worker_threads);
	clear_structures();
	printf("Memory cleared.\n");

	return 0;
}