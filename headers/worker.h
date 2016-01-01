#ifndef WORKER_H
#define WORKER_H

#include <stdint.h>

#include "structures.h"

#define SIZE_CHECK_JOB 1
#define DEFRAGMENT_JOB 2
#define GO_HOME 3

typedef struct {
	uint32_t worker_id;
	char** cluster_buffer;
	uint32_t* def_indexes;
	root_directory** rds;
} worker;

#endif