#ifndef WORKER_FUNCTIONS_H
#define WORKER_FUNCTIONS_H

#include "structures.h"

/**
Function that is being passed to worker threads. Worker asks for jobs in the get_job function
and in case it return a number greater or equal than 0, it will use that number as an index
into the list with record_directories. It also decides based on the current_job variable,
what job will be done - file size check or defragmentation. Concrete function are thne called based
on that parameter.
*/
void* do_work(uint32_t current_job);

/**
Function that decides, what file will be assign to a worker thread asking for job.
If there are none (all files were serviced), then it returns -1. It uses mutex so no two workers
cas access the variable in the same time.
*/
uint64_t get_job();
	
/**
Function that checks for lenght of a file described in root_directory given as an input parameterer.
It first calculates expected number of slusters from file_size variable in root_direcotry and cluster_size
from boot_record. Then it loops through FAT table checking indexes of clusters belonging to the file
(starting at first_cluster index from root_directory) and increments real_cluster_count variable.
If real and expected number of clusters are equal, then function returns 0, otherwise 1.
*/
uint32_t size_check_func(root_directory* rd);

/**
Function for incrementing bad_file_size_sum global variable. It employs mutex,
so only one worker thread can access it at any given time.
*/
void increment_bad_file_size_sum(uint32_t increment_cnt);

suspend_cluster* defragment_func(root_directory* rd, uint32_t start_index, suspend_cluster* s_cluster);

void defragment_suspended_func(suspend_cluster* s_cluster_start);

uint32_t cluster_unused(uint32_t cluster_index);

void clear_suspended_cluster_list(suspend_cluster* s_cluster_list);

#endif