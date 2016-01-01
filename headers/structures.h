/*
Header file containg structures for storing information about FAT file - Boot Record and Root Directory.
Code and constants for these structures were taken from materials to semestral project from KIV/ZOS subject.
*/

#ifndef STRUCUTRES_H
#define STRUCTURES_H

#include <stdint.h>

#define FAT_UNUSED 65535
#define FAT_FILE_END 65534
#define FAT_BAD_CLUSTER 65533

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

char* input_file_path; //string with path to the input file
char* output_file_path; //string with path to the output file
uint32_t threads_cnt; //maximum number of workers that will be created to do stuff
uint32_t job_type; //what type of job will be workers doing

boot_record* br; //structure for storing Boot Record in the memory
uint32_t** fat_tables; //structure for storing FAT tables in the memory
root_directory** rd_list; //structure for storing individual Root Directories in the memory
char** clusters; //strucure for storing clusters with their content in the memory

uint64_t processed_files; //variable for storing how many have been processed so far by workers
uint32_t* defrag_indexes; //structure for saving first indexes of individuial files that will be used for defragmenation 

#endif