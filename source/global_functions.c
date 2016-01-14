#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>

#include "structures.h"
#include "global_functions.h"

void check_arguments(uint32_t argc, char** argv) {
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


uint32_t save_file(char* file) {
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


void clear_structures() {
	uint32_t i;
	
	//clearing first indexes of files used for defragmentation
	free(defrag_indexes);
	
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


void prepare_fat_tables_for_defrag() {
	uint32_t i, j;
	uint64_t file_cluster_count;
	uint32_t cluster_index;
	
	defrag_indexes = malloc(sizeof(uint32_t) * br->root_directory_max_entries_count);
	cluster_index = 0;
	
	for(i = 0; i < br->root_directory_max_entries_count; i++) {
		file_cluster_count =  ceil((double) rd_list[i]->file_size / (double) br->cluster_size);
		
		for(j = cluster_index; j < (cluster_index + file_cluster_count); j++) {
			if(clusters[j] == FAT_BAD_CLUSTER) {
				cluster_index = j + 1;
				j = cluster_index - 1;
			}
		}
		
		defrag_indexes[i] = cluster_index;		
		cluster_index += file_cluster_count;
	}
}