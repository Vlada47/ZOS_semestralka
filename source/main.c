#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <math.h>
#include <pthread.h>

#include "structures.h"
#include "worker.h"

pthread_mutex_t get_job_mutex = PTHREAD_MUTEX_INITIALIZER;

/*
Function for checking parameters given to the program.
It controls correct number of arguments and whether the value for number of threads
is numerical and greater than zero.
*/
void check_arguments(int argc, char** argv) {
	if(argc != 4) {
		printf("Program needs to be given 3 parameters - path to the input file, output file and number of available threads.\n");
		exit(EXIT_FAILURE);
	}
	else {
		input_file_path = argv[1];
		output_file_path = argv[2];
		
		if((threads_cnt = atoi(argv[3])) <= 0) {
			printf("You have to pass positive integer value as the number of available threads.\n");
			exit(EXIT_FAILURE);
		}
	}
}

/*
Function that loads contents of the file into prepared global structures.
First it loads Boot Record that is needed for some values like cluster size or count
so we know how many of what structures we will need to allocate memory for.
If file can't be opened, function exits the program with EXIT_FAILURE code.
*/
void load_file(char* file) {
	uint32_t i;
	FILE *fat_file;
	
	if((fat_file = fopen(file, "rb"))) {
		//loading Boot Record
		br = (boot_record *) malloc(sizeof(boot_record));
		fread(br, sizeof(boot_record), 1, fat_file);
		
		//loading FAT tables
		fat_tables = malloc(sizeof(uint32_t*) * br->fat_copies);
		for (i = 0; i < br->fat_copies; i++) {
			fat_tables[i] = malloc(sizeof(uint32_t) * br->cluster_count);
			fread(fat_tables[i], sizeof(uint32_t), br->cluster_count, fat_file);
		}
		
		//loading Root Directories
		rd_list = malloc(sizeof(root_directory*) * br->root_directory_max_entries_count);
		for(i = 0; i < br->root_directory_max_entries_count; i++) {
			rd_list[i] = malloc(sizeof(root_directory));
			fread(rd_list[i], sizeof(root_directory), 1, fat_file);
		}
		
		//loading clusters
		clusters = malloc(sizeof(char*) * br->cluster_count);
		for(i = 0; i < br->cluster_count; i++) {
			clusters[i] = malloc(br->cluster_size);
			fread(clusters[i], br->cluster_size, 1, fat_file);
		}
		
		fclose(fat_file);
	}
	else {
		printf("Can't open the file %s.\n", file);
		exit(EXIT_FAILURE);
	}
}

/*
Function for saving content (after needed operations) into file.
It will be in the same structure as file that was read from.
*/
int save_file(char* file) {
	uint32_t i;
	FILE *fat_file;
	
	if((fat_file = fopen(file, "wb"))) {
		
		//saving Boot Record
		fwrite(br, sizeof(boot_record), 1, fat_file);
		
		//saving FAT tables
		for(i = 0; i < br->fat_copies; i++) {
			fwrite(fat_tables[i] , sizeof(uint32_t), br->cluster_count, fat_file);
		}
		
		//saving Root Directories
		for(i = 0; i < br->root_directory_max_entries_count; i++) {
			fwrite(rd_list[i], sizeof(root_directory), 1, fat_file);
		}
		
		//saving clusters
		for(i = 0; i < br->cluster_count; i++) {
			fwrite(clusters[i], br->cluster_size, 1, fat_file);
		}
		
		fclose(fat_file);
		return 0;
	}
	else {
		printf("Can't open the file %s.\n", file);
		return 1;
	}
}

/*
This function prints the content of structures for storing content of the file.
It shouldn't be used before the file was loaded in those structures.
*/
void print_structures() {
	uint32_t i, j;
	//printing Boot Record
	printf("-------------------------------------------------------- \n");
    printf("BOOT RECORD \n");
    printf("-------------------------------------------------------- \n");
    printf("volume_descriptor :%s\n",br->volume_descriptor);
  	printf("fat_type :%d\n",br->fat_type);
  	printf("fat_copies :%d\n",br->fat_copies);
  	printf("cluster_size :%d\n",br->cluster_size);
  	printf("root_directory_max_entries_count :%ld\n",br->root_directory_max_entries_count);    
  	printf("cluster count :%d\n",br->cluster_count);
  	printf("reserved clusters :%d\n",br->reserved_cluster_count);
  	printf("signature :%s\n",br->signature);
	
	//printing FAT tables
	printf("-------------------------------------------------------- \n");
    printf("FAT \n");
    printf("-------------------------------------------------------- \n");
	for(i = 0; i < br->fat_copies; i++) {
		printf("\nFAT COPIES %d\n", i + 1);
		
		for(j = 0; j < br->cluster_count; j++) {
			if (fat_tables[i][j] != FAT_UNUSED){
				if (fat_tables[i][j] == FAT_FILE_END)
					printf("%d - FILE_END\n", j);
				else if (fat_tables[i][j] == FAT_BAD_CLUSTER)
					printf("%d - BAD_CLUSTER\n", j);
				else
					printf("%d - %d\n", j, fat_tables[i][j]);
            
			}  
		}
	}
	
	//printing Root Directories
	printf("-------------------------------------------------------- \n");
    printf("ROOT DIRECTORY \n");
    printf("-------------------------------------------------------- \n");
	for(i = 0; i < br->root_directory_max_entries_count; i++) {
		printf("FILE %d \n",i);
		printf("file_name :%s\n",rd_list[i]->file_name); 
		printf("file_mod :%s\n",rd_list[i]->file_mod);
		printf("file_type :%d\n",rd_list[i]->file_type);
		printf("file_size :%ld\n",rd_list[i]->file_size);
		printf("first_cluster :%d\n",rd_list[i]->first_cluster);
	}
	
	//printing clusters
	printf("-------------------------------------------------------- \n");
    printf("CLUSTERY - OBSAH \n");
    printf("-------------------------------------------------------- \n");
	for(i = 0; i < br->cluster_count; i++) {
		if(clusters[i][0] != '\0')
			printf("Cluster %d:%s\n",i,clusters[i]);
	}
}

/*
Function for clearing the content of global structures.
It should be used at the end of the program.
*/
void clear_structures() {
	uint32_t i;
	
	//clearing clusters
	for(i = 0; i < br->cluster_count; i++) {
		free(clusters[i]);
	}
	free(clusters);
	
	//clearing Root Directories
	for(i = 0; i < br->root_directory_max_entries_count; i++) {
		free(rd_list[i]);
	}
	free(rd_list);
	
	//clearing FAT tables
	for(i = 0; i < br->fat_copies; i++) {
		free(fat_tables[i]);
	}
	free(fat_tables);
	
	//clearing Boot Record
	free(br);
}

/*
Function for creating a list with indexes that will be used for moving content of files.
It basically denotes, which clusters will be used as first clusters of individual files (one index per file/Root Directory). 
*/
void prepare_fat_tables_for_defrag() {
	uint32_t i, j;
	uint64_t file_cluster_count;
	uint32_t cluster_index;
	
	defrag_indexes = malloc(sizeof(uint32_t) * br->root_directory_max_entries_count);
	cluster_index = 0;
	
	for(i = 0; i < br->root_directory_max_entries_count; i++) {
		file_cluster_count =  ceil((double) rd_list[i]->file_size / (double) br->cluster_size);
		defrag_indexes[i] = cluster_index;
		cluster_index += file_cluster_count;
	}
}

void initiate_worker_job(void *(*job_func)(), uint32_t thread_number) {
	worker my_workers[thread_number];
	pthread_t my_threads[thread_number];
	uint32_t i;
	
	for(i = 0; i < thread_number; i++) {
		my_workers[i].worker_id = i;
		pthread_create(&(my_threads[i]), NULL, job_func, (void *)(&my_workers[i]));
	}
}

uint32_t get_next_job(worker* w) {
	pthread_mutex_lock(&get_job_mutex);
	
	if(processed_files < br->root_directory_max_entries_count) {
		add_new_job(w, rd_list[processed_files], defrag_indexes[processed_files]);
		processed_files++;
		pthread_mutex_unlock(&get_job_mutex);
		return job_type;
	}
	else {
		pthread_mutex_unlock(&get_job_mutex);
		return GO_HOME;
	}
} 

int main(int argc, char** argv){
	uint64_t i;
	
	printf("\n");
	
	check_arguments(argc, argv);
	printf("Arguments OK.\n");
	printf("\n");
	
	load_file(input_file_path);
	printf("File loaded into memory.\n");
	printf("\n");
	
	print_structures();
	printf("End of contents.\n");
	printf("\n");
	
	processed_files = 0;
	
	//work of threads
	
	if((save_file(output_file_path) == 0)) {
		printf("File was successfully saved.\n");
	}
	else {
		printf("File couldn't be saved.\n");
	}
	printf("\n");
	
	clear_structures();
	printf("Memory cleared.\n");

	return 0;
}
