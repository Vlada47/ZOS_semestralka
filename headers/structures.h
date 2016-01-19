/**
Header file containg structures for storing information about FAT file - Boot Record and Root Directory.
Code and constants for these structures were taken from materials to semestral project from KIV/ZOS subject.
It also contains all global variables and structures that are used and manipulated in the main code + all constants.
*/

#ifndef STRUCUTRES_H
#define STRUCTURES_H

#include <stdint.h>

#define FAT_UNUSED 65535
#define FAT_FILE_END 65534
#define FAT_BAD_CLUSTER 65533

#define CHECK_FILE_SIZE_JOB 1
#define DEFRAGMENATATION_JOB 2
#define CLUSTER_MUTEX_CNT 10

typedef struct {
    char volume_descriptor[251];     
    int32_t fat_type;                             
    int32_t fat_copies;                           
    uint32_t cluster_size;                
    int64_t root_directory_max_entries_count;    
    uint32_t cluster_count;               
    uint32_t reserved_cluster_count;      
    char signature[4];                        
} boot_record;

typedef struct {
    char file_name[13];          
    char file_mod[10]; 
    int16_t file_type;
    int64_t file_size;
    uint32_t first_cluster;
} root_directory;

/**
This structure is for holding content of clusters, which cannot be moved during defragmentation task right away.
It also contains index, where it will be placed, and index that will be used as a value in FAT table. 
*/
typedef struct suspend_cluster {
	char* cluster_content;
	uint32_t target_index;
	uint32_t target_index_next;
	struct suspend_cluster* next;
} suspend_cluster;

extern char* input_file_path; //string with path to the input file
extern char* output_file_path; //string with path to the output file
extern uint32_t threads_cnt; //maximum number of workers that will be created to do stuff

extern boot_record* br; //structure for storing Boot Record in the memory
extern uint32_t** fat_tables; //structure for storing FAT tables in the memory
extern root_directory** rd_list; //structure for storing individual Root Directories in the memory
extern char** clusters; //strucure for storing clusters with their content in the memory

extern uint64_t processed_files; //variable for storing how many have been processed so far by workers
extern uint32_t bad_file_size_sum; //variable for storing of how many files had wrong size
extern uint32_t* defrag_indexes; //structure for saving first indexes of individuial files that will be used for defragmenation 

extern pthread_mutex_t get_job_mutex; //mutex for job asigning function
extern pthread_mutex_t bad_file_size_sum_mutex; //mutex for incrementing variable denoting how many files have bad size
extern pthread_mutex_t cluster_mutex_array[CLUSTER_MUTEX_CNT]; //array of mutexes for clusters (for use during defragmentation)

#endif