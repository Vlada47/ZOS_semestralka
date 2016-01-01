#include "worker.h"

void do_stuff() {
//loop, where worker asks for jobs
}

void add_new_job(worker* w, root_directory* rd, uint32_t def_index) {
//storing all needed information in stack structures of the worker	
}











/*
Function that checks for lenght of a file described in root_directory given as an input parameterer.
It first calculates expected number of slusters from file_size variable in root_direcotry and cluster_size
from boot_record. Then it loops through FAT table checking indexes of clusters belonging to the file
(starting at first_cluster index from root_directory) and increments real_cluster_count variable.
If real and expected number of clusters are equal, then function returns 0, otherwise 1.
*/
int size_check_func(root_directory* rd, worker* my_worker) {
	long exp_cluster_count, real_cluster_count;
	int cluster_index;
	
	exp_cluster_count = ceil ((double) rd->file_size / (double) br->cluster_size);
	real_cluster_count = 1;
	cluster_index = fat_tables[0][rd->first_cluster];
	
	while(cluster_index != FAT_FILE_END) {
		cluster_index = fat_tables[0][cluster_index];
		real_cluster_count++;
	}
	
	if(exp_cluster_count == real_cluster_count) {
		printf("File %s has correct length of %ld clusters.\n", rd->file_name, real_cluster_count);
		return 0;
	}
	else {
		printf("File %s has incorrect length. Was expecting %ld clusters, but it had %ld clusters.\n", rd->file_name, exp_cluster_count, real_cluster_count);
		return 1;
	}
}