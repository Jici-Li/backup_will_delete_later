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
uint32_t L2_cache_total_accesses = 0;
uint32_t L2_cache_hits = 0;
uint32_t L2_cache_misses = 0;
uint32_t L2_cache_read_accesses = 0;
uint32_t L2_cache_read_hits = 0;
uint32_t L2_cache_write_accesses = 0;
uint32_t L2_cache_write_hits = 0;
uint32_t memory_total_accesses = 0;
uint32_t memory_read_accesses = 0;
uint32_t memory_write_accesses = 0;

// Input parameters to control the cache.
uint32_t cache_level = 1;
uint32_t L1_cache_size = 4096;
uint32_t L1_cache_associativity = 1;
uint32_t L1_cache_block_size = 4;
uint32_t L2_cache_size = 65536;
uint32_t L2_cache_associativity = 1;
uint32_t L2_cache_block_size = 4;

block_t ***cache;
block_t ***L2_cache;

void update_lru(uint32_t index, uint32_t accessed_way);
void update_lru_L2(uint32_t index, uint32_t accessed_way);
int find_lru_way(uint32_t index);
int find_lru_way_L2(uint32_t index);
int read_from_L2_cache_real(uint32_t pa);
int write_to_L2_cache_real(uint32_t pa);
void install_to_L2_cache(uint32_t pa); 

int read_from_memory(uint32_t pa) { return 0; }

int write_to_memory(uint32_t pa) { return 0; }


int read_from_L2_cache(uint32_t pa) { 
    return read_from_L2_cache_real(pa);
}

int write_to_L2_cache(uint32_t pa) { 
    return write_to_L2_cache_real(pa);
}

void initialize_cache() { 
    int num_blocks = 0;
    int num_sets = 0;
    if (L1_cache_block_size != 0 && L1_cache_associativity != 0) {
        num_blocks = L1_cache_size / L1_cache_block_size;
        num_sets = num_blocks / L1_cache_associativity;
    }
    if (num_sets == 0) num_sets = 1;

    cache = malloc(num_sets * sizeof(block_t**));
    for (int i = 0; i < num_sets; i++) {
        cache[i] = malloc(L1_cache_associativity * sizeof(block_t*));
        for (int j = 0; j < L1_cache_associativity; j++) {
            cache[i][j] = malloc(sizeof(block_t));
            cache[i][j]->valid = false;
            cache[i][j]->dirty = false;
            cache[i][j]->tag = 0;
            cache[i][j]->lru_counter = 0;
        }
    }

    if (cache_level == 2) {
        L2_cache_size = L1_cache_size * 16;
        L2_cache_associativity = L1_cache_associativity;
        L2_cache_block_size = L1_cache_block_size;
        
        int L2_num_blocks = 0;
        int L2_num_sets = 0;
        if (L2_cache_block_size != 0 && L2_cache_associativity != 0) {
            L2_num_blocks = L2_cache_size / L2_cache_block_size;
            L2_num_sets = L2_num_blocks / L2_cache_associativity;
        }
        if (L2_num_sets == 0) L2_num_sets = 1;

        L2_cache = malloc(L2_num_sets * sizeof(block_t**));
        for (int i = 0; i < L2_num_sets; i++) {
            L2_cache[i] = malloc(L2_cache_associativity * sizeof(block_t*));
            for (int j = 0; j < L2_cache_associativity; j++) {
                L2_cache[i][j] = malloc(sizeof(block_t));
                L2_cache[i][j]->valid = false;
                L2_cache[i][j]->dirty = false;
                L2_cache[i][j]->tag = 0;
                L2_cache[i][j]->lru_counter = 0;
            }
        }
    }
}

void free_cache() {
    int num_blocks = 0;
    int num_sets = 0;
    if (L1_cache_block_size != 0 && L1_cache_associativity != 0) {
        num_blocks = L1_cache_size / L1_cache_block_size;
        num_sets = num_blocks / L1_cache_associativity;
    }
    if (num_sets == 0) num_sets = 1;
    
    for (int i = 0; i < num_sets; i++) {
        for (int j = 0; j < L1_cache_associativity; j++) {
            free(cache[i][j]);
        }
        free(cache[i]);
    }
    free(cache);

    if (cache_level == 2) {
        int L2_num_blocks = 0;
        int L2_num_sets = 0;
        if (L2_cache_block_size != 0 && L2_cache_associativity != 0) {
            L2_num_blocks = L2_cache_size / L2_cache_block_size;
            L2_num_sets = L2_num_blocks / L2_cache_associativity;
        }
        if (L2_num_sets == 0) L2_num_sets = 1;
        
        for (int i = 0; i < L2_num_sets; i++) {
            for (int j = 0; j < L2_cache_associativity; j++) {
                free(L2_cache[i][j]);
            }
            free(L2_cache[i]);
        }
        free(L2_cache);
    }
}

void update_lru(uint32_t index, uint32_t accessed_way) {
    for (int way = 0; way < L1_cache_associativity; way++) {
        if (cache[index][way]->valid) {
            if (way == accessed_way) 
                cache[index][way]->lru_counter = 0;
            else 
                cache[index][way]->lru_counter++;
        }
    }
}

void update_lru_L2(uint32_t index, uint32_t accessed_way) {
    for (int way = 0; way < L2_cache_associativity; way++) {
        if (L2_cache[index][way]->valid) {
            if (way == accessed_way) 
                L2_cache[index][way]->lru_counter = 0;
            else 
                L2_cache[index][way]->lru_counter++;
        }
    }
}

int find_lru_way(uint32_t index) {
    int lru_way = 0;
    uint32_t max_counter = 0;
    for (int way = 0; way < L1_cache_associativity; way++) {
        if (!cache[index][way]->valid) return way;
        if (cache[index][way]->lru_counter > max_counter) {
            max_counter = cache[index][way]->lru_counter;
            lru_way = way;
        }
    }
    return lru_way;
}

int find_lru_way_L2(uint32_t index) {
    int lru_way = 0;
    uint32_t max_counter = 0;
    for (int way = 0; way < L2_cache_associativity; way++) {
        if (!L2_cache[index][way]->valid) return way;
        if (L2_cache[index][way]->lru_counter > max_counter) {
            max_counter = L2_cache[index][way]->lru_counter;
            lru_way = way;
        }
    }
    return lru_way;
}

int read_from_L2_cache_real(uint32_t pa) {
    L2_cache_total_accesses++;
    L2_cache_read_accesses++;

    int L2_num_blocks = 0;
    int L2_num_sets = 0;
    if (L2_cache_block_size != 0 && L2_cache_associativity != 0) {
        L2_num_blocks = L2_cache_size / L2_cache_block_size;
        L2_num_sets = L2_num_blocks / L2_cache_associativity;
    }
    if (L2_num_sets == 0) L2_num_sets = 1;

    uint32_t index = (pa / L2_cache_block_size) % L2_num_sets;
    uint32_t tag = pa / (L2_cache_block_size * L2_num_sets);

    for (int way = 0; way < (int)L2_cache_associativity; way++) {
        if (L2_cache[index][way]->valid && L2_cache[index][way]->tag == tag) {
            update_lru_L2(index, way);
            L2_cache_hits++;
            L2_cache_read_hits++;
            return 1; 
        }
    }
    
    L2_cache_misses++;
    return 0;
}

int write_to_L2_cache_real(uint32_t pa) {
    L2_cache_total_accesses++;
    L2_cache_write_accesses++;

    int L2_num_blocks = 0;
    int L2_num_sets = 0;
    if (L2_cache_block_size != 0 && L2_cache_associativity != 0) {
        L2_num_blocks = L2_cache_size / L2_cache_block_size;
        L2_num_sets = L2_num_blocks / L2_cache_associativity;
    }
    if (L2_num_sets == 0) L2_num_sets = 1;

    uint32_t index = (pa / L2_cache_block_size) % L2_num_sets;
    uint32_t tag = pa / (L2_cache_block_size * L2_num_sets);

    for (int way = 0; way < (int)L2_cache_associativity; way++) {
        if (L2_cache[index][way]->valid && L2_cache[index][way]->tag == tag) {
            L2_cache[index][way]->dirty = true;
            update_lru_L2(index, way);
            L2_cache_hits++;
            L2_cache_write_hits++;
            return 1;
        }
    }

    L2_cache_misses++;
    int replace_way = find_lru_way_L2(index);

    if (L2_cache[index][replace_way]->valid && L2_cache[index][replace_way]->dirty) {
        memory_write_accesses++;
        memory_total_accesses++;
    }

    L2_cache[index][replace_way]->valid = true;
    L2_cache[index][replace_way]->tag = tag;
    L2_cache[index][replace_way]->dirty = true;
    L2_cache[index][replace_way]->lru_counter = 0;
    
    update_lru_L2(index, replace_way);
    return 0; 
}


void install_to_L2_cache(uint32_t pa) {
    int L2_num_blocks = L2_cache_size / L2_cache_block_size;
    int L2_num_sets = L2_num_blocks / L2_cache_associativity;
    if (L2_num_sets == 0) L2_num_sets = 1;
    
    uint32_t index = (pa / L2_cache_block_size) % L2_num_sets;
    uint32_t tag = pa / (L2_cache_block_size * L2_num_sets);
    
    int replace_way = find_lru_way_L2(index);
    if (L2_cache[index][replace_way]->valid && L2_cache[index][replace_way]->dirty) {
        memory_write_accesses++;
        memory_total_accesses++;
    }
    L2_cache[index][replace_way]->valid = true;
    L2_cache[index][replace_way]->tag = tag;
    L2_cache[index][replace_way]->dirty = false; 
    L2_cache[index][replace_way]->lru_counter = 0;
    
    update_lru_L2(index, replace_way);
}

op_result_t read_from_cache(uint32_t pa) {
    L1_cache_total_accesses++;
    L1_cache_read_accesses++;

    int num_blocks = 0;
    int num_sets = 0;
    if (L1_cache_block_size != 0 && L1_cache_associativity != 0) {
        num_blocks = L1_cache_size / L1_cache_block_size;
        num_sets = num_blocks / L1_cache_associativity;
    }
    if (num_sets == 0) num_sets = 1;

    uint32_t index = (pa / L1_cache_block_size) % num_sets;
    uint32_t tag = pa / (L1_cache_block_size * num_sets);

    bool hit = false;
    int hit_way = -1;
    for (int way = 0; way < (int)L1_cache_associativity; way++) {
        if (cache[index][way]->valid && cache[index][way]->tag == tag) {
            hit = true;
            hit_way = way;
            break;
        }
    }

    if (hit) {
        update_lru(index, hit_way);
        L1_cache_hits++;
        L1_cache_read_hits++;
        return HIT;
    } else {
        L1_cache_misses++;

        if (cache_level == 2) {
            if (read_from_L2_cache(pa)) {
                int replace_way = find_lru_way(index);
          
                if (cache[index][replace_way]->valid && cache[index][replace_way]->dirty) {

                  uint32_t victim_pa = (cache[index][replace_way]->tag * (num_sets * L1_cache_block_size)) + (index * L1_cache_block_size);
                  write_to_L2_cache(victim_pa);
                }

                cache[index][replace_way]->valid = true;
                cache[index][replace_way]->tag = tag;
                cache[index][replace_way]->dirty = false;
                cache[index][replace_way]->lru_counter = 0;
                update_lru(index, replace_way);
                install_to_L2_cache(pa);
            } else {
                memory_total_accesses++;
                memory_read_accesses++;

                int replace_way = find_lru_way(index);
                 if (cache[index][replace_way]->valid && cache[index][replace_way]->dirty) {
                    uint32_t victim_pa = (cache[index][replace_way]->tag * (num_sets * L1_cache_block_size)) + (index * L1_cache_block_size);
                    write_to_L2_cache(victim_pa);
                }

                cache[index][replace_way]->valid = true;
                cache[index][replace_way]->tag = tag;
                cache[index][replace_way]->dirty = false;
                cache[index][replace_way]->lru_counter = 0;
                update_lru(index, replace_way);
            }
        } else {
            memory_total_accesses++;
            memory_read_accesses++;

            int replace_way = find_lru_way(index);
            if (cache[index][replace_way]->valid && cache[index][replace_way]->dirty) {
                memory_write_accesses++;
                memory_total_accesses++;
            }

            cache[index][replace_way]->valid = true;
            cache[index][replace_way]->tag = tag;
            cache[index][replace_way]->dirty = false;
            cache[index][replace_way]->lru_counter = 0;
            update_lru(index, replace_way);
        }
        return MISS;
    }
}

op_result_t write_to_cache(uint32_t pa) {
    L1_cache_total_accesses++;
    L1_cache_write_accesses++;

    int num_blocks = 0;
    int num_sets = 0;
    if (L1_cache_block_size != 0 && L1_cache_associativity != 0) {
        num_blocks = L1_cache_size / L1_cache_block_size;
        num_sets = num_blocks / L1_cache_associativity;
    }
    if (num_sets == 0) num_sets = 1;

    uint32_t index = (pa / L1_cache_block_size) % num_sets;
    uint32_t tag = pa / (L1_cache_block_size * num_sets);

    bool hit = false;
    int hit_way = -1;
    for (int way = 0; way < (int)L1_cache_associativity; way++) {
        if (cache[index][way]->valid && cache[index][way]->tag == tag) {
            hit = true;
            hit_way = way;
            break;
        }
    }

    if (hit) {
        cache[index][hit_way]->dirty = true;
        update_lru(index, hit_way);
        L1_cache_hits++;
        L1_cache_write_hits++;
        return HIT;
    } else {
        L1_cache_misses++;

        if (cache_level == 2) {
            if (read_from_L2_cache(pa)) {
                int replace_way = find_lru_way(index);
                  if (cache[index][replace_way]->valid && cache[index][replace_way]->dirty) {
                    uint32_t victim_pa = (cache[index][replace_way]->tag * (num_sets * L1_cache_block_size)) + (index * L1_cache_block_size);
                    write_to_L2_cache(victim_pa);
                }
                
                cache[index][replace_way]->valid = true;
                cache[index][replace_way]->tag = tag;
                cache[index][replace_way]->dirty = true;
                cache[index][replace_way]->lru_counter = 0;
                update_lru(index, replace_way);
            } else {
                memory_total_accesses++;
                memory_read_accesses++;
                
                int replace_way = find_lru_way(index);
                 if (cache[index][replace_way]->valid && cache[index][replace_way]->dirty) {
                    uint32_t victim_pa = (cache[index][replace_way]->tag * (num_sets * L1_cache_block_size)) + (index * L1_cache_block_size);
                    write_to_L2_cache(victim_pa);
                }
                
                cache[index][replace_way]->valid = true;
                cache[index][replace_way]->tag = tag;
                cache[index][replace_way]->dirty = true;
                cache[index][replace_way]->lru_counter = 0;
                update_lru(index, replace_way);

                install_to_L2_cache(pa);
            }
        } else {
            int replace_way = find_lru_way(index);
            if (cache[index][replace_way]->valid && cache[index][replace_way]->dirty) {
                memory_write_accesses++;
                memory_total_accesses++;
            }

            memory_read_accesses++;
            memory_total_accesses++;

            cache[index][replace_way]->valid = true;
            cache[index][replace_way]->tag = tag;
            cache[index][replace_way]->dirty = true;
            cache[index][replace_way]->lru_counter = 0;
            update_lru(index, replace_way);
        }
        return MISS;
    }
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

int process_arg_S(int opt, char *optarg) {
    L1_cache_size = atoi(optarg);
    if (L1_cache_size <= 0) return 1;
    if ((L1_cache_size & (L1_cache_size - 1)) != 0) return 1;
    return 0;
}

int process_arg_A(int opt, char *optarg) { 
    L1_cache_associativity = atoi(optarg); 
    return 0; 
}

int process_arg_B(int opt, char *optarg) { 
    L1_cache_block_size = atoi(optarg); 
    return 0; 
}

int process_arg_L(int opt, char *optarg) { 
    cache_level = atoi(optarg); 
    return 0; 
}

int process_arg_P(int opt, char *optarg) { 
    return 0; 
}

static int is_power_of_two(uint32_t x) {
    return x != 0 && ((x & (x - 1)) == 0);
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
