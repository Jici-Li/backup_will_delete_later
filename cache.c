#include "cache.h"
#include <stdlib.h>

// Cache statistics counters.
uint32_t L1_cache_total_accesses = 0;
uint32_t L1_cache_hits = 0;
uint32_t L1_cache_misses = 0;
uint32_t L1_cache_read_accesses = 0;
uint32_t L1_cache_read_hits = 0;
uint32_t L1_cache_write_accesses = 0;
uint32_t L1_cache_write_hits = 0;
uint32_t L2_cache_total_accesses;
uint32_t L2_cache_hits;
uint32_t L2_cache_misses;
uint32_t L2_cache_read_accesses;
uint32_t L2_cache_read_hits;
uint32_t L2_cache_write_accesses;
uint32_t L2_cache_write_hits;
uint32_t memory_total_accesses = 0;
uint32_t memory_read_accesses = 0;
uint32_t memory_write_accesses = 0;

// Input parameters to control the cache.
uint32_t cache_level=1;
uint32_t L1_cache_size=4096;
uint32_t L1_cache_associativity=1;
uint32_t L1_cache_block_size=4;
uint32_t L2_cache_size;
uint32_t L2_cache_associativity;
uint32_t L2_cache_block_size;

block_t ***cache;
void update_lru(uint32_t index,uint32_t accessed_way);
int find_lru_way(uint32_t index);

/*
 * Perform a read from the memory for a particular address.
 * Since this is a cache-simulation, memory is not involved.
 * No need to implement the logic for this function.
 * Call this function when cache needs to read from memory.
 */
int read_from_memory(uint32_t pa) { 
  return 0;
  
}

/*
 * Perform a write from the cache to memory for a particular address.
 * Since this is a cache-simulation, memory is not involved.
 * No need to implement the logic for this function.
 * Call this function when cache needs to write to memory.
 */
int write_to_memory(uint32_t pa) {
  return 0;
}

/*
 * Perform a read from the L2_cache for a particular address.
 * Again, no need to implement the logic for this function.
 * Call this function when the L1 cache needs to read from the L2 cache.
 */
int read_from_L2_cache(uint32_t pa) { return 0; }

/*
 * Perform a write from the L2 cache for a particular address.
 * Again, no need to implement the logic for this function.
 * Call this function when the L1 cache needs to write to the L2 cache.
 */
int write_to_L2_cache(uint32_t pa) { return 0; }

/*
 *********************************************************
  Please edit the below functions.
  You are allowed to add more functions that may help you
  with your implementation. However, please do not add
  them in any file. All your new code should be in cache.c
  file below this line.
 *********************************************************
*/

/*
 * Initialize the cache depending on the input parameters S, A, and B
 * and the statistics counter. The cache is declared in as extern in
 * include/cache.h file.
 */
void initialize_cache() { 
  int num_blocks=(L1_cache_size/L1_cache_block_size);
  int num_sets=(num_blocks/L1_cache_associativity);

  cache=malloc(num_sets*sizeof(block_t**));

  for(int i=0;i<num_sets;i++){
  cache[i]=malloc(L1_cache_associativity*sizeof(block_t*));

  for(int j=0;j<L1_cache_associativity;j++){
    cache[i][j]=malloc(sizeof(block_t));
    cache[i][j]->valid=false;
    cache[i][j]->dirty=false;
    cache[i][j]->tag=0;
    cache[i][j]->lru_counter=0;
   }
  }
}

/*
 * Free the allocated memory for the cache to avoid memory leaks.
 */
void free_cache() {
  int num_blocks=(L1_cache_size/L1_cache_block_size);
  int num_sets=(num_blocks/L1_cache_associativity);

  for(int i=0;i<num_sets;i++){
    for(int j=0;j<L1_cache_associativity;j++){
      free(cache[i][j]);
    }
  
      free(cache[i]);
    }
      free(cache);
    }

void update_lru(uint32_t index, uint32_t accessed_way) {
    for (int way = 0; way < L1_cache_associativity; way++) {
        if (cache[index][way]->valid) {
            if (way == accessed_way) {
                cache[index][way]->lru_counter = 0;
            } else {
                cache[index][way]->lru_counter++;
            }
        }
    }
}

int find_lru_way(uint32_t index) {
    int lru_way = 0;
    uint32_t max_counter = 0;
    
    for (int way = 0; way < L1_cache_associativity; way++) {
        if (!cache[index][way]->valid) {
            return way;
        }
        if (cache[index][way]->lru_counter > max_counter) {
            max_counter = cache[index][way]->lru_counter;
            lru_way = way;
        }
    }
    return lru_way;
}

// Print cache statistics.
void print_cache_statistics() {
  printf("\n* Cache Statistics *\n");

  printf("memory total accesses: %d\n", memory_total_accesses);
  printf("memory read accesses: %d\n", memory_read_accesses);
  printf("memory write accesses: %d\n", memory_write_accesses);

  printf("L1 total accesses: %d\n", L1_cache_total_accesses);
  printf("L1 hits: %d\n", L1_cache_hits);
  printf("L1 misses: %d\n", L1_cache_misses);
  printf("L1 total reads: %d\n", L1_cache_read_accesses);
  printf("L1 read hits: %d\n", L1_cache_read_hits);
  printf("L1 total writes: %d\n", L1_cache_write_accesses);
  printf("L1 write hits: %d\n", L1_cache_write_hits);

  if (cache_level == 2) {
    printf("L2 total accesses: %d\n", L2_cache_total_accesses);
    printf("L2 hits: %d\n", L2_cache_hits);
    printf("L2 misses: %d\n", L2_cache_misses);
    printf("L2 total reads: %d\n", L2_cache_read_accesses);
    printf("L2 read hits: %d\n", L2_cache_read_hits);
    printf("L2 total writes: %d\n", L2_cache_write_accesses);
    printf("L2 write hits: %d\n", L2_cache_write_hits);
  }
}

/*
 * Perform a read from the cache for a particular address.
 * Since this is a simulation, there is no data. Ignore the data part.
 * The return value is always a HIT or a MISS and never an ERROR.
 */

op_result_t read_from_cache(uint32_t pa) { 
  L1_cache_total_accesses++;      
  L1_cache_read_accesses++;//whatever it is,when see one new address, access+1
  int hit_way=-1;

  //get the variables needed 
  int num_blocks=L1_cache_size/L1_cache_block_size;
  int num_sets=num_blocks/L1_cache_associativity;

  //all valid with 2's complement
  uint32_t index=(pa/L1_cache_block_size)%num_sets;//(pa(the size of the cache)/block size=the number of blocks)/number of sets=how many blocks in a set=index
  uint32_t tag=pa/(L1_cache_block_size*num_sets);//pa-(index+offset)=tag

  bool hit=false;
  for(int way=0;way<L1_cache_associativity; way++){
    if(cache[index][way]->valid&&cache[index][way]->tag==tag){
      hit=true;
      hit_way=way;
      break;
    }
  }//only can use when both tag in corresponse and is valid
  
  if(hit){
    update_lru(index,hit_way);
    L1_cache_hits++;
    L1_cache_read_hits++;
    return HIT;

  }else{
    L1_cache_misses++;
    memory_total_accesses++;
    memory_read_accesses++;
    int replace_way=find_lru_way(index);
    cache[index][replace_way]->valid=true;
    cache[index][replace_way]->tag=tag;
    cache[index][replace_way]->dirty=false;
    cache[index][replace_way]->lru_counter=0;

    update_lru(index, replace_way);
    
    return MISS;
  }
}

/*
 * Perform a write from the cache for a particular address.
 * Since this is a simulation, there is no data. Ignore the data part.
 * The return value is always a HIT or a MISS and never an ERROR.
 */
op_result_t write_to_cache(uint32_t pa) {
  L1_cache_total_accesses++;
  L1_cache_write_accesses++;
  int hit_way=-1;

  int num_blocks=L1_cache_size/L1_cache_block_size;
  int num_sets=num_blocks/L1_cache_associativity; 

  uint32_t index=(pa/L1_cache_block_size)%num_sets;
  uint32_t tag=pa/(L1_cache_block_size*num_sets);

  bool hit=false;
  for(int way=0;way<L1_cache_associativity; way++){
    if(cache[index][way]->valid&&cache[index][way]->tag==tag){
      hit=true;
      cache[index][way]->dirty=true;  
      hit_way=way;
      break;
    }
  }
  if(hit){
    update_lru(index,hit_way);
    L1_cache_hits++;
    L1_cache_write_hits++;
    return HIT;

  }else{
    L1_cache_misses++;
    memory_total_accesses++;
    memory_read_accesses++; 
    int replace_way=find_lru_way(index);
    cache[index][replace_way]->valid=true;
    cache[index][replace_way]->tag=tag;
    cache[index][replace_way]->dirty=true;
    cache[index][replace_way]->lru_counter=0;  
    
    update_lru(index, replace_way);  
    
    return MISS;
  }
}

// Process the S parameter properly and initialize `cache_size`.
// Return 0 when everything is good. Otherwise return -1.
int process_arg_S(int opt, char *optarg) { 
  L1_cache_size=atoi(optarg);
  return 0; }

// Process the A parameter properly and initialize `cache_associativity`.
// Return 0 when everything is good. Otherwise return -1.
int process_arg_A(int opt, char *optarg) { 
  L1_cache_associativity=atoi(optarg);
  return 0; }

// Process the B parameter properly and initialize `cache_block_size`.
// Return 0 when everything is good. Otherwise return -1.
int process_arg_B(int opt, char *optarg) { 
  L1_cache_block_size=atoi(optarg);
  return 0; }

// Process the L parameter properly and initialize `cache_level`.
// Return 0 when everything is good. Otherwise return -1.
int process_arg_L(int opt, char *optarg) { 
  cache_level=atoi(optarg);
  return 0; }

// When verbose is true, print the details of each operation -- MISS or HIT.
void handle_cache_verbose(memory_access_entry_t entry, op_result_t ret) {
  if (ret == ERROR) {
    printf("This message should not be printed. Fix your code\n");
  }
}

// Check if all the necessary paramaters for the cache are provided and valid.
// Return 0 when everything is good. Otherwise return -1.
int check_cache_parameters_valid() { return 0; }

// *********************************************************
// BONUS TASK ONLY:
// Process the P parameter properly and initialize `cache_prefetching_policy`.
// Return 0 when everything is good. Otherwise return -1.
// *********************************************************
int process_arg_P(int opt, char *optarg) { return 0; }
