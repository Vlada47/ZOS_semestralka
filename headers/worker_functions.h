#ifndef WORKER_FUNCTIONS_H
#define WORKER_FUNCTIONS_H

#include <stdint.h>

/**
Function that is being passed to worker threads. Worker asks for jobs in the get_job function
and in case it returns a number greater or equal than 0, it will use that number as an index
into the list with root_directory structures. It also decides based on the current_job variable,
what job will be done - file size check or defragmentation. Concrete function are then called based
on that parameter.
After depleting all available jobs, workers complete defragmentation by moving clusters from
suspended cluster list to correct places.

current_job - type of the job workers will be doing: file size check / defragmentation
*/
void* do_work(void* current_job);

/**
Function that decides, what file will be assign to a worker thread asking for job.
If there are none (all files were serviced - (when it reaches root_directory_max_entries_count)),
then it returns -1. It uses get_job_mutex so no two workers can access the variable processed_files at the same time.

returns - index into root_directory list or -1
*/
int64_t get_job();
	
/**
Function that checks for lenght of a file described in root_directory given as an input parameterer.
It first calculates expected number of clusters from file_size variable in root_directory and cluster_size
from boot_record. Then it loops through FAT table checking indexes of clusters belonging to the file
(starting at first_cluster index from root_directory) and increments real_cluster_count variable.
If real and expected number of clusters are equal, then function returns 0, otherwise 1.

rd - pointer to root_directory structure describing the checked file

returns - 0 if file has correct size, 1 if file has incorrect size
*/
uint32_t size_check_func(root_directory* rd);

/**
Function for incrementing bad_file_size_sum global variable. It is used to sum return values
from size_check_func functions. It employs bad_file_size_sum_mutex so only one worker thread can access it at any given time.

increment_cnt - number that will used to increment bad_file_size_sum variable
*/
void increment_bad_file_size_sum(uint32_t increment_cnt);

/**
Function for defragmentation task. It saves first_index variable from rd structure and replaces its value with
start_index variable (the index, from which will the file will be placed). It then loops for each cluster in the file,
while locking currently serviced clusters, checking if there is FAT_UNUSED value on clusters that will be used as a place
for defragmented clusters.
If yes, then cluster will be placed and FAT table records changed accordingly. If no, serviced cluster will be added to
suspend_cluster list that will be solved later. In both cases the value on current index (from which the cluster was just taken)
in FAT table will be set to FAT_UNUSED so other files can have it free for their clusters.

rd - pointer to root_directory structure describing the defragmented file
start_index - index from which will be file starting after defragmentation
s_cluster - pointer to last record in the suspend_cluster list for the worker that holds it

return - pointer to last record in the suspend_cluster list for the worker that holds it
*/
suspend_cluster* defragment_func(root_directory* rd, uint32_t start_index, suspend_cluster* s_cluster);

/**
Function for defragmentation of suspended clusters (that couldn't be placed during previous operation due to still used places).
It checks if there are actually any such clusters and if yes, it will check, whether they are free to place with cluster_unused() function.
The worker will yield while this is not true. It then places the cluster to correct place and change values in FAT table accordingly.
It uses mutexes to prevent moving clusters that are used by other workers.

s_cluster_start - pointer to first record in suspend_cluster list
*/
void defragment_suspended_func(suspend_cluster* s_cluster_start);

/**
Function that checks if cluster on specific index if free to be used.

cluster_index - index of the cluster
*/
uint32_t cluster_unused(uint32_t cluster_index);

/**
Function that will free the memory used by suspend_cluster list on the worker.

s_cluster_list - pointer to first record in suspend_cluster list
*/
void clear_suspended_cluster_list(suspend_cluster* s_cluster_list);

#endif