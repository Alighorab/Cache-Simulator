#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <getopt.h>
#include <math.h>
#include <limits.h>

#include "cachelab.h"

typedef struct cache_line {
    unsigned valid;
    unsigned tag;
    unsigned long long time; /* Time stamp */ 
} CacheLine, *CacheLinePtr;

typedef struct cache {
    unsigned s;      /* Number of sets bits */
    unsigned S;      /* Number of sets */
    unsigned E;      /* Associativity */
    unsigned b;      /* Number block size bits (in bytes) */
    unsigned B;      /* Number of block size in bytes */
    CacheLine **start;
} Cache, *CachePtr; 

typedef struct state {
    unsigned hits;
    unsigned misses;
    unsigned evictions;
} State, *StatePtr;

void help();
int parse(CachePtr cp, char** trace_file, int argc, char* argv[]);
void create_cache(CachePtr cp);
void simulate(CachePtr cp, char* trace_filename, StatePtr sp, unsigned verb);
void sim_load_store(CachePtr cp, unsigned address, StatePtr sp, unsigned verb);
void sim_modify(CachePtr cp, unsigned address, StatePtr sp, unsigned verb);
void sim_direct_mapped(CachePtr cp, unsigned index, unsigned tag, unsigned verb
                        ,StatePtr state);
void sim_set_associative(CachePtr cp, unsigned index, unsigned tag, 
                            unsigned verb, StatePtr state);
void evict(CachePtr cp, unsigned index, unsigned tag);
void free_cache(CachePtr cp);

static unsigned time = 0;

void
help()
{
    int c;
    FILE *file = NULL;
    file = fopen("help", "r");
    if (file) {
        while ((c = fgetc(file)) != EOF)
            putchar(c);
        fclose(file);
    }
}

int
parse(CachePtr cp, char** trace_file, int argc, char* argv[])
{
    int opt = 0;
    int verbose = 0;
    int len = 0;
    while ((opt = getopt(argc, argv, "hvs:E:b:t:")) != -1) {
        switch (opt) {
        case 'h':
            help();
            exit(-1);
        case 'v':
            verbose = 1;
            break;
        case 's':
            cp->s = atoi(optarg);
            cp->S = pow(2, cp->s);
            break;
        case 'E':
            cp->E = atoi(optarg);
            break;
        case 'b':
            cp->b = atoi(optarg);
            cp->B = pow(2, cp->b);
            break;
        case 't':
            len = strlen(optarg);
            if (trace_file) {
                *trace_file = malloc(len + 1);
                if (*trace_file) {            
                    strncpy(*trace_file, optarg, len);
                }   
            }
            break;
        default:
            help();
            exit(-1);
        }
    }
    return verbose;
}

void
create_cache(CachePtr cp)
{
    /* create cache as a two-dimentional array */
    CacheLine **cache = (CacheLine**) calloc(cp->S, sizeof(CacheLine*));
    if (cache) {
        int i = 0;
        for (i = 0; i < cp->S; i++) {
            cache[i] = (CacheLine*) calloc(cp->E, sizeof(CacheLine));
            if (!cache[i]) {
                printf("faild to allocate cache\n");
                exit(-1);
            }
        }
    } else {
        printf("faild to allocate cache\n");
        exit(-1);
    }
    cp->start = cache;
}

void
simulate(CachePtr cp, char* trace_filename, StatePtr sp, unsigned verb)
{
    char id = 0; 
    unsigned address = 0, size = 0;
    FILE* trace_file = fopen(trace_filename, "r");

    while (fscanf(trace_file, " %c %x,%u", &id, &address, &size) > 0) {
        switch (id) {
        case 'L': /* FALLTHROUGH */
        case 'S':
            if (verb == 1) {
                printf("%c %x,%u ", id, address, size);
            }
            sim_load_store(cp, address, sp, verb);
            break;
        case 'M':
            if (verb == 1) {
                printf("%c %x,%u ", id, address, size);
            }
            sim_modify(cp, address, sp, verb);
            break;
        default:
            ; /* Do nothing for instruction load operation */
        }
    }
    fclose(trace_file);
}

void
sim_load_store(CachePtr cp, unsigned address, StatePtr sp, unsigned verb)
{
    /* extract tag and set index from address */
    unsigned index = (address & (cp->S * cp->B - 1)) >> cp->b;
    unsigned tag = address >> (cp->s + cp->b);

    if (cp->E == 1) {
        sim_direct_mapped(cp, index, tag, verb, sp);
        if (verb == 1) {
            printf("\n");
        }
    } else {
        sim_set_associative(cp, index, tag, verb, sp);
        if (verb == 1) {
            printf("\n");
        }
    }
}


void
sim_modify(CachePtr cp, unsigned address, StatePtr sp, unsigned verb)
{
    /* extract tag and set index from address */
    unsigned index = (address & (cp->S * cp->B - 1)) >> cp->b;
    unsigned tag = address >> (cp->s + cp->b);

    if (cp->E == 1) {
        sim_direct_mapped(cp, index, tag, verb, sp);
        sim_direct_mapped(cp, index, tag, verb, sp);
        if (verb == 1) {
            printf("\n");
        }

    } else {
        sim_set_associative(cp, index, tag, verb, sp);
        sim_set_associative(cp, index, tag, verb, sp);
        if (verb == 1) {
            printf("\n");
        }
    }
}

void
sim_direct_mapped(CachePtr cp, unsigned index, unsigned tag, unsigned verb,
                        StatePtr sp)
{
    if (cp->start[index][0].valid == 1) {
        if (cp->start[index][0].tag == tag) {
            sp->hits++;
            if (verb == 1) {
                printf("%s", "hit ");
            } 
        } else {
            sp->misses++;
            sp->evictions++;
            cp->start[index][0].tag = tag;

            if (verb == 1) {
                printf("%s", "miss eviction ");
            } 

        }
    } else {
        sp->misses++;
        cp->start[index][0].valid = 1;
        cp->start[index][0].tag = tag;
        if (verb == 1) {
            printf("%s", "miss ");
        } 

    }
}


void
sim_set_associative(CachePtr cp, unsigned index, unsigned tag, unsigned verb,
                        StatePtr sp)
{
    int i;
    unsigned lines = cp->E;
    for (i = 0; i < lines; i++) {
        if (cp->start[index][i].valid == 1) {
            if (cp->start[index][i].tag == tag) {
                sp->hits++;
                cp->start[index][i].time = time++;
                if (verb == 1) {
                    printf("%s", "hit ");
                } 
                return;
            }
        } else {
            sp->misses++;
            cp->start[index][i].valid = 1;
            cp->start[index][i].tag = tag;
            cp->start[index][i].time = time++;
            
            if (verb == 1) {
                printf("%s", "miss ");
            } 
            return;
        }
    }
    sp->misses++;
    sp->evictions++;
    evict(cp, index, tag);
    if (verb == 1) {
        printf("%s", "miss eviction ");
    } 

}

void
evict(CachePtr cp, unsigned index, unsigned tag)
{
    /* LRU algorithm */ 
    int lru = 0;
    int t = cp->start[index][0].time;
    int i;
    for (i = 1; i < cp->E; i++) {
        if (cp->start[index][i].time < t) {
            t = cp->start[index][i].time;
            lru = i;
        }
    }
    cp->start[index][lru].tag = tag;
    cp->start[index][lru].time = time++;
}


void 
free_cache(CachePtr cp)
{
    int i;
    for (i = 0; i < cp->S; i++) {
        free(cp->start[i]);
        cp->start[i] = NULL;
    }
    free(cp->start);
    cp->start = NULL;
}


int
main(int argc, char* argv[])
{
    /* 0. Local variables */
    Cache cache = {0, 0, 0, 0, 0, NULL};
    State state = {0, 0, 0};
    unsigned verbose = 0;
    char* trace_filename = NULL;

    if (argc < 2 || argc < 9) {
        help();
        exit(-1);
    }

    /* 1. Parse input */
    verbose = parse(&cache, &trace_filename, argc, argv);

    /* 2. Creare cache */
    create_cache(&cache);

    /* 3. Read trace file and simulate */
    simulate(&cache, trace_filename, &state, verbose);

    /* 4. Free memory */
    free_cache(&cache); 
    free(trace_filename);
    trace_filename = NULL;

    /* 5. Print summary */
    printSummary(state.hits, state.misses, state.evictions);
    
    return 0;
}
