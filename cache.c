#include "cache.h"
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

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

block_t ***cache_L1;
block_t ***cache_L2;
void update_lru(uint32_t index,uint32_t accessed_way);
int find_lru_way(uint32_t index);

/*
 * Perform a read from the memory for a particular address.
 * Since this is a cache-simulation, memory is not involved.
 * No need to implement the logic for this function.
 * Call this function when cache needs to read from memory.
 */
int read_from_memory(uint32_t pa) { return 0; }
/*
 * Perform a write from the cache to memory for a particular address.
 * Since this is a cache-simulation, memory is not involved.
 * No need to implement the logic for this function.
 * Call this function when cache needs to write to memory.
 */
int write_to_memory(uint32_t pa) { return 0; }
/*
 * Perform a read from the L2_cache for a particular address.
 * Again, no need to implement the logic for this function.
 * Call this function when the L1 cache needs to read from the L2 cache.
 */
int read_from_L2_cache(uint32_t pa) { return 0;

}
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
    int num_blocks=0;
    int num_sets=0;
    if(L1_cache_block_size!=0&&L1_cache_associativity!=0) {
        num_blocks=L1_cache_size/L1_cache_block_size;
        num_sets=num_blocks/L1_cache_associativity;
    }
    if(num_sets==0)num_sets=1;

    cache_L1=malloc(num_sets*sizeof(block_t**));
    for(int i=0;i<num_sets;i++){
        cache_L1[i]=malloc(L1_cache_associativity*sizeof(block_t*));
        for(int j=0;j<L1_cache_associativity;j++){
            cache_L1[i][j] = malloc(sizeof(block_t));
            cache_L1[i][j]->valid = false;
            cache_L1[i][j]->dirty = false;
            cache_L1[i][j]->tag = 0;
            cache_L1[i][j]->lru_counter = 0;
        }
    }
}

void free_cache() {
    int num_blocks=0;
    int num_sets=0;
    if (L1_cache_block_size!=0&&L1_cache_associativity!=0) {
        num_blocks=L1_cache_size/L1_cache_block_size;
        num_sets=num_blocks/L1_cache_associativity;
    }
    if (num_sets==0)num_sets=1;
    for(int i=0;i<num_sets;i++){
        for(int j=0;j<L1_cache_associativity;j++){
            free(cache_L1[i][j]);
        }
        free(cache_L1[i]);
    }
    free(cache_L1);
}

void initialize_cache() { 
    int num_blocks=0;
    int num_sets=0;
    if(L1_cache_block_size!=0&&L1_cache_associativity!=0) {
        num_blocks=L1_cache_size/L1_cache_block_size;
        num_sets=num_blocks/L1_cache_associativity;
    }
    if(num_sets==0)num_sets=1;

    cache_L2=malloc(num_sets*sizeof(block_t**));
    for(int i=0;i<num_sets;i++){
        cache_L2[i]=malloc(L1_cache_associativity*sizeof(block_t*));
        for(int j=0;j<L1_cache_associativity;j++){
            cache_L2[i][j] = malloc(sizeof(block_t));
            cache_L2[i][j]->valid = false;
            cache_L2[i][j]->dirty = false;
            cache_L2[i][j]->tag = 0;
            cache_L2[i][j]->lru_counter = 0;
        }
    }
}

void free_cache() {
    int num_blocks=0;
    int num_sets=0;
    if(L1_cache_block_size!=0&&L1_cache_associativity!=0){
        num_blocks=L1_cache_size/L1_cache_block_size;
        num_sets=num_blocks/L1_cache_associativity;
    }
    if(num_sets==0)num_sets=1;
    for(int i=0;i<num_sets;i++){
        for(int j=0;j<L1_cache_associativity;j++){
            free(cache_L2[i][j]);
        }
        free(cache_L2[i]);
    }
    free(cache_L2);
}

void update_lru(uint32_t index,uint32_t accessed_way) {
    for (int way=0;way<L1_cache_associativity;way++) {
        if (cache[index][way]->valid) {
            if (way==accessed_way) cache[index][way]->lru_counter=0;
            else cache[index][way]->lru_counter++;
        }
    }
}

int find_lru_way(uint32_t index) {
    int lru_way=0;
    uint32_t max_counter=0;
    for(int way=0; way<L1_cache_associativity; way++){
        if(!cache[index][way]->valid) return way;
        if(cache[index][way]->lru_counter > max_counter){
            max_counter=cache[index][way]->lru_counter;
            lru_way=way;
        }
    }
    return lru_way;
}

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

/* Read from L1 */
op_result_t read_from_cache(uint32_t pa) {
    L1_cache_total_accesses++;
    L1_cache_read_accesses++;

    int num_blocks=0;
    int num_sets=0;
    if (L1_cache_block_size!=0&&L1_cache_associativity!=0){
        num_blocks=L1_cache_size/L1_cache_block_size;
        num_sets=num_blocks/L1_cache_associativity;
    }
    if (num_sets==0)num_sets=1;

    uint32_t index=(pa/L1_cache_block_size)%num_sets;
    uint32_t tag=pa/(L1_cache_block_size*num_sets);

    bool hit=false;
    int hit_way=-1;
    for (int way=0;way<(int)L1_cache_associativity;way++){
        if (cache[index][way]->valid&&cache[index][way]->tag==tag){
            hit=true;
            hit_way=way;
            break;
        }
    }

    if(hit){
        update_lru(index,hit_way);
        L1_cache_hits++;
        L1_cache_read_hits++;
        return HIT;
    }else{
        L1_cache_misses++;

        // MISS -> need to bring block from memory
        memory_total_accesses++;
        memory_read_accesses++;

        int replace_way=find_lru_way(index);

        // if victim is valid and dirty->writeback first
        if (cache[index][replace_way]->valid && cache[index][replace_way]->dirty){
            memory_write_accesses++;    // write-back to memory
            memory_total_accesses++;
        }

        cache[index][replace_way]->valid=true;
        cache[index][replace_way]->tag=tag;
        cache[index][replace_way]->dirty=false;
        cache[index][replace_way]->lru_counter=0;

        update_lru(index, replace_way);
        return MISS;
    }
}

op_result_t write_to_cache(uint32_t pa){
    L1_cache_total_accesses++;
    L1_cache_write_accesses++;

    int num_blocks=0;
    int num_sets=0;
    if (L1_cache_block_size != 0&&L1_cache_associativity!=0) {
        num_blocks = L1_cache_size / L1_cache_block_size;
        num_sets = num_blocks / L1_cache_associativity;
    }
    if (num_sets==0) num_sets=1;

    uint32_t index=(pa/L1_cache_block_size)%num_sets;
    uint32_t tag=pa/(L1_cache_block_size*num_sets);

    bool hit=false;
    int hit_way=-1;
    for (int way=0; way<(int)L1_cache_associativity; way++){
        if (cache[index][way]->valid && cache[index][way]->tag==tag){
            hit=true;
            hit_way=way;
            break;
        }
    }

    if (hit){
        cache[index][hit_way]->dirty=true;
        update_lru(index,hit_way);
        L1_cache_hits++;
        L1_cache_write_hits++;
        return HIT;
    }else{
        L1_cache_misses++;

        int replace_way=find_lru_way(index);

        if (cache[index][replace_way]->valid && cache[index][replace_way]->dirty){
            memory_write_accesses++;
            memory_total_accesses++;
        }

        memory_read_accesses++;       
        memory_total_accesses++;

        cache[index][replace_way]->valid=true;
        cache[index][replace_way]->tag=tag;
        cache[index][replace_way]->dirty=true;
        cache[index][replace_way]->lru_counter=0;

        update_lru(index,replace_way);
        return MISS;
    }
}

int process_arg_S(int opt, char *optarg){L1_cache_size=atoi(optarg); 
 L1_cache_size=atoi(optarg);
    if (L1_cache_size<=0) 
    return 1;
    if ((L1_cache_size&(L1_cache_size-1))!=0) 
    return 1;

    return 0;
}
int process_arg_A(int opt, char *optarg) { L1_cache_associativity = atoi(optarg); return 0; }
int process_arg_B(int opt, char *optarg) { L1_cache_block_size = atoi(optarg); return 0; }
int process_arg_L(int opt, char *optarg) { cache_level = atoi(optarg); return 0; }
int process_arg_P(int opt, char *optarg) { return 0; }

static int is_power_of_two(uint32_t x) {
    return x != 0 && ( (x & (x - 1)) == 0 );
}

int check_cache_parameters_valid() {
    if (L1_cache_size == 0) return -1;
    if (L1_cache_block_size == 0) return -1;
    if (L1_cache_associativity == 0) return -1;

    if (L1_cache_size % L1_cache_block_size != 0) return -1;

    int blocks = L1_cache_size / L1_cache_block_size;
    if (blocks % L1_cache_associativity != 0) return -1;

    if (!is_power_of_two(L1_cache_size)) return -1;
    if (!is_power_of_two(L1_cache_block_size)) return -1;

    return 0;
}

void handle_cache_verbose(memory_access_entry_t entry, op_result_t ret) {
    if (ret == ERROR) {
        printf("This message should not be printed. Fix your code\n");
    }
}
