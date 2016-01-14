#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>

#include "structures.h"
#include "worker_functions.h"

void* do_work(uint32_t current_job) {
	uint64_t my_job;
	
	suspend_cluster* s_cluster_start = (suspend_cluster*) malloc(sizeof(suspend_cluster));
	suspend_cluster* s_cluster_end = s_cluster_start;
	suspend_cluster* s_cluster_defrag;
	
	s_cluster_start->cluster_content = "\n";
	s_cluster_start->target_index = -1;
	s_cluster_start->target_index_next = -1;
	s_cluster_start->next = NULL;
	
	while((my_job = get_job()) >= 0) {
		switch(current_job) {
			case CHECK_FILE_SIZE_JOB:
				increment_bad_file_size_sum(size_check_func(rd_list[my_job]));
				break;
			case DEFRAGMENATATION_JOB:
				s_cluster_end = defragment_func(rd_list[my_job], defrag_indexes[my_job], s_cluster_end);
				break;
		}
	}
	
	if(s_cluster_start->next != NULL) {
		defragment_suspended_func(s_cluster_start);
	}
	
	clear_suspended_cluster_list(s_cluster_start);
}


uint64_t get_job() {
	uint64_t file_id;
	pthread_mutex_lock(&get_job_mutex);
	
	if(processed_files < br->root_directory_max_entries_count) {
		file_id = processed_files;
		processed_files++;
		pthread_mutex_unlock(&get_job_mutex);
		return file_id;
	}
	else {
		pthread_mutex_unlock(&get_job_mutex);
		return -1;
	}
}


uint32_t size_check_func(root_directory* rd) {
	uint64_t exp_cluster_count, real_cluster_count;
	uint32_t cluster_index;
	
	exp_cluster_count = ceil ((double) rd->file_size / (double) br->cluster_size);
	real_cluster_count = 1;
	cluster_index = fat_tables[0][rd->first_cluster];
	
	while(cluster_index != FAT_FILE_END && real_cluster_count <= exp_cluster_count) {
		cluster_index = fat_tables[0][cluster_index];
		real_cluster_count++;
	}
	
	if(cluster_index == FAT_FILE_END) {
		printf("File %s has correct size.\n", rd->file_name);
		return 0;
	}
	else {
		printf("File %s has incorrect size.\n", rd->file_name);
		return 1;
	}
}


void increment_bad_file_size_sum(uint32_t increment_cnt) {
	pthread_mutex_lock(&bad_file_size_sum_mutex);
	bad_file_size_sum += increment_cnt;
	pthread_mutex_unlock(&bad_file_size_sum_mutex);
}


suspend_cluster* defragment_func(root_directory* rd, uint32_t start_index, suspend_cluster* s_cluster) {
	uint64_t i, j;
	uint32_t curr_index;
	uint32_t next_index;
	uint64_t cluster_count;
	
	curr_index = rd->first_cluster;
	cluster_count = ceil ((double) rd->file_size / (double) br->cluster_size);
	
	for(i = 0; i < cluster_count; i++) {
		//locked
		if(((start_index + i) % CLUSTER_MUTEX_CNT) == (curr_index % CLUSTER_MUTEX_CNT)) {
			pthread_mutex_lock(&(cluster_mutex_array[curr_index % CLUSTER_MUTEX_CNT]));
		}
		else {
			pthread_mutex_lock(&(cluster_mutex_array[curr_index % CLUSTER_MUTEX_CNT]));
			pthread_mutex_lock(&(cluster_mutex_array[(start_index + i) % CLUSTER_MUTEX_CNT]));
		}
		
		if(fat_tables[0][start_index + i] == FAT_UNUSED) {
			clusters[start_index + i] = clusters[curr_index];
			
			next_index = fat_tables[0][curr_index];
			
			for(j = 0; j < br->fat_copies; j++) {
				fat_tables[j][curr_index] = FAT_UNUSED;
				
				if(i == cluster_count-1) {
					fat_tables[j][start_index + i] = FAT_FILE_END;
				}
				else {
					fat_tables[j][start_index + i] = (start_index + i) + 1;
				}
			}
		}
		else {
			s_cluster->next = (suspend_cluster*) malloc(sizeof(suspend_cluster));
			s_cluster = s_cluster->next;
			s_cluster->cluster_content = clusters[curr_index];
			s_cluster->target_index = start_index + i;
			s_cluster->next = NULL;
			
			if(i == cluster_count-1) {
				s_cluster->target_index_next = FAT_FILE_END;
			}
			else {
				s_cluster->target_index_next = (start_index + i) + 1;
			}
			
			next_index = fat_tables[0][curr_index];
			
			for(j = 0; j < br->fat_copies; j++) {
				fat_tables[j][curr_index] = FAT_UNUSED;
			}
		}
		
		if(((start_index + i) % CLUSTER_MUTEX_CNT) == (curr_index % CLUSTER_MUTEX_CNT)) {
			pthread_mutex_unlock(&(cluster_mutex_array[curr_index % CLUSTER_MUTEX_CNT]));
		}
		else {
			pthread_mutex_unlock(&(cluster_mutex_array[curr_index % CLUSTER_MUTEX_CNT]));
			pthread_mutex_unlock(&(cluster_mutex_array[(start_index + i) % CLUSTER_MUTEX_CNT]));
		}
		//unlocked
		
		curr_index = next_index;
	}
	
	return s_cluster;
}


void defragment_suspended_func(suspend_cluster* s_cluster_start) {
	uint32_t i;
	suspend_cluster* s_cluster_curr = s_cluster_start;
	
	while(s_cluster_curr != NULL) {
		
		while(!(cluster_unused(s_cluster_curr->target_index))) {
			pthread_yield();
		}
		
		//locked
		pthread_mutex_lock(&(cluster_mutex_array[s_cluster_curr->target_index % CLUSTER_MUTEX_CNT]));
		
		clusters[s_cluster_curr->target_index] = s_cluster_curr->cluster_content;
		
		for(i = 0; i < br->fat_copies; i++) {
			fat_tables[i][s_cluster_curr->target_index] = s_cluster_curr->target_index_next;
		}
		
		pthread_mutex_unlock(&(cluster_mutex_array[s_cluster_curr->target_index % CLUSTER_MUTEX_CNT]));
		//unlocked
		
		s_cluster_curr = s_cluster_curr->next;
	}
}


uint32_t cluster_unused(uint32_t cluster_index) {
	uint32_t x;
	
	pthread_mutex_lock(&(cluster_mutex_array[cluster_index % CLUSTER_MUTEX_CNT]));
	x = fat_tables[0][cluster_index];
	pthread_mutex_unlock(&(cluster_mutex_array[cluster_index % CLUSTER_MUTEX_CNT]));
	
	return (x == FAT_UNUSED);
}


void clear_suspended_cluster_list(suspend_cluster* s_cluster_list) {
	suspend_cluster* curr = s_cluster_list;
	suspend_cluster* last;
	
	while(curr != NULL) {
		last = curr;
		curr = curr->next;
		free(last);
	}
}