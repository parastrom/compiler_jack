#ifndef SAFER_H
#define SAFER_H

#include <stdlib.h>
#include "logger.h"
#include "vector.h"

void* safer_malloc(size_t size);
void safer_free(void* ptr);
unsigned int hash(const char* str, size_t size);
char* read_file_into_string(const char* filename);
#endif // SAFER_H
#ifndef SAFER_H
#define SAFER_H

#include <stdlib.h>
#include "logger.h"

void* safer_malloc(size_t size);
void safer_free(void* ptr);
unsigned int hash(const char* str, size_t size);

#endif // SAFER_H
