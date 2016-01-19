#ifndef GLOBAL_FUNCTIONS_H
#define GLOBAL_FUNCTIONS_H

#include <stdint.h>

/**
Function for checking parameters given to the program.
It controls correct number of arguments and whether the value for number of threads
is numerical and greater than zero.

argc - count of arguments
argv - an array with arguments
*/
void check_arguments(uint32_t argc, char** argv);

/**
Function that loads contents of the file into prepared global structures.
First it loads Boot Record that is needed for some values like cluster size or count
so we know how many of what structures we will need to allocate memory for.
If file can't be opened, function exits the program with EXIT_FAILURE code.

file - path to input file
*/
void load_file(char* file);

/**
Function for saving content (after needed operations) into file.
It will be in the same structure as file that was read from.

file - path to outpus file (will be created there)

return - 0 if file was saved, 1 if not
*/
uint32_t save_file(char* file);

/**
This function prints the content of structures for storing content of the file.
It shouldn't be used before the file was loaded in those structures.
*/
void print_structures();

/**
Function for clearing the content of global structures.
It should be used at the end of the program.
*/
void clear_structures();

/**
Function for creating a list with indexes that will be used for moving content of files.
It basically denotes, which clusters will be used as first clusters of individual files (one index per file/Root Directory). 
It loops through all files, calculating their size and determining, where they will start.
If it encounters a FAT_BAD_CLUSTER, it will move the index behind it. Indexes are stored in defrag_indexes list (global structure).
*/
void prepare_fat_tables_for_defrag();

#endif