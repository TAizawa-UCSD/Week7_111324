#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <sys/mman.h>

#define HEAP_SIZE 400
#define VAL_SIZE 8

uint64_t* HEAP_START = NULL;

void init_heap(){
  //Hey operating system, where can I start my heap?
  
  uint64_t *heap = mmap(NULL, HEAP_SIZE, PROT_READ | PROT_WRITE, MAO_ANON | MAP_SHARED, -1, 0);//mmap means "Hey OS, I want heap memory, give me a bunch and tell me where it started
  HEAP_START = heap;
}

void* my_malloc(size_t size){
  sdf
}

int main(){
  int* a = my_malloc(40);
  int* b = my_malloc(10);
}
