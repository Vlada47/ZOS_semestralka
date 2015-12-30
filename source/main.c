#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>

#include "structures.h"


char* file_path; //string with path to the file
uint32_t threads_cnt; //maximum number of workers that will be created to do stuff

boot_record* br; //structure for storing Boot Record in the memory
uint32_t** fat_tables; //structure for storing FAT tables in the memory
root_directory** rd_list; //structure for storing individual Root Directories in the memory
char** clusters; //strucure for storing clusters with their content in the memory

/*
Function for checking parameters given to the program.
It controls correct number of arguments and whether the value for number of threads
is numerical and greater than zero.
*/
void check_arguments(int argc, char** argv) {
	if(argc != 3) {
		printf("Program needs to be given 2 parameters - path to the file and number of available threads.\n");
		exit(EXIT_FAILURE);
	}
	else {
		file_path = argv[1];
		
		if((threads_cnt = atoi(argv[2])) <= 0) {
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
	int i;
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
This function prints the content of structures for storing content of the file.
It shouldn't be used before the file was loaded in those structures.
*/
void print_structures() {
	int i, j;
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
		printf("\nFAT KOPIE %d\n", i + 1);
		
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
	int i;
	
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

int main(int argc, char** argv){
	
	check_arguments(argc, argv);
	printf("Arguments OK.\n");
	
	load_file(file_path);
	printf("File loaded into memory.\n");
	
	print_structures();
	printf("End of contents.\n");
	
	clear_structures();
	printf("Memory cleared.\n");
	
	return 0;
}
