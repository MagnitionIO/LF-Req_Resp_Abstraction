#ifndef ___MAP_H___
#define ___MAP_H___

#include <stdbool.h>
#include <stdint.h>
#include "utils.h"

#define HASH_SEED   53

#define MAX_MAP_NAME_LEN    100

typedef enum {
    NODE_UNUSED = 0,
    NODE_ALLOCATED,
    NODE_FREED
} node_state;

typedef struct map_node {
    node_state state;
    void *key;
    void *value;
} map_node;

typedef struct map_t {
    size_t size;
    size_t allocated_count;
    size_t freed_count;
    double load_threshold;
    map_node *elements;
    char name[MAX_MAP_NAME_LEN];
    uint32_t uuid;
    uint32_t map_id;
} map_t;

bool init_map (map_t *map, size_t _size, double _load_threshold, const char *_name, uint32_t _uuid, uint32_t _map_id);
bool resize_if_needed (map_t *map);
bool insert (map_t *map, void *key, void *_value);
bool _insert (map_t *map, void *key, void *_value);
void *erase (map_t *map, void *key);
void *find (map_t *map, void *key);
bool is_map_empty (map_t *map);
void dump_map (map_t *map);

/*
#define _CALLOC(len,size,type) \
({ \
    type * allocation = (type *)calloc (len, size); \
    log_trace ("_CALLOC:%s ptr:%p", type, allocation); \
    allocation; \
})

#define CALLOC(len,size,ref) _CALLOC(len,size,typeof(*ref));
*/

#define _CALLOC(len,size) \
({ \
    void *allocation = calloc (len, size); \
    log_trace ("_CALLOC:%p", allocation); \
    allocation; \
})

#define _FREE(ptr) \
do { \
    log_trace ("_FREE:%p", ptr); \
    free (ptr); \
} while(0)

#endif