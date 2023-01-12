#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "map.h"

bool init_map (map_t *map, size_t _size, double _load_threshold, const char *_name, uint32_t _uuid, uint32_t _map_id) {
    memset (map, 0, sizeof (map_t));
    map->uuid = _uuid;
    map->map_id = _map_id;
    map->size = _size;
    map->load_threshold = _load_threshold;
    map->elements = calloc (_size, sizeof(map_node));
    if (map->elements == NULL) {
        log_error ("Out of memory, could not allocate map entries");
        assert (map->elements);
    }
    unsigned int ret = snprintf (map->name, MAX_MAP_NAME_LEN, "%s:%u:%u", _name, map->uuid, map->map_id);
    if (ret >= MAX_MAP_NAME_LEN) {
        log_error ("Map name too long");
        map->name[MAX_MAP_NAME_LEN - 1] = 0;
    }
    return true;
}

bool resize_if_needed (map_t *map) {
    if ((map->allocated_count + map->freed_count) >= ((double)map->size * map->load_threshold)) {
        map_node *current_elements = map->elements;
        ssize_t current_size = map->size;
        map->size <<= 1;
        map->elements = calloc(map->size, sizeof(map_node));
        if (map->elements == NULL) {
            log_error ("Out of memory, could not allocate map entries");
            assert (map->elements);
        }
        map->allocated_count = map->freed_count = 0;
        for (size_t i = 0; i < current_size; ++i) {
            if (current_elements[i].state != NODE_ALLOCATED) {
                continue;
            }
            _insert(map, current_elements[i].key, current_elements[i].value);
        }
        free(current_elements);
    }
    return true;
}

bool _insert (map_t *map, void *key, void *_value) {
    size_t index = (HASH_SEED * ((size_t) key)) % map->size;
    map_node *elem_list = map->elements;
    bool found_freed_slot = false;
    size_t insert_index = index;

    for (size_t itr = 0; itr < map->size; ++itr, index = (index + 1) % map->size) {
        log_debug ( "Iterating [%zu] state:%d %s key:%p value:%p",
                    index, elem_list[index].state, elem_list[index].value ? elem_list[index].value : "NULL", elem_list[index].key, elem_list[index].value);
        if (elem_list[index].state == NODE_UNUSED) {
            break;
        }
        else if (elem_list[index].state == NODE_ALLOCATED) {
            if (elem_list[index].key == key) {
                log_warn ("Adding entry:%p that is already in map", key);
                return false;
            }
        } else if (elem_list[index].state == NODE_FREED) {
            if (found_freed_slot == false) {
                insert_index = index;
                found_freed_slot = true;
            }
        }
    }

    insert_index = found_freed_slot ? insert_index : index;
    assert ((insert_index >= 0) && (insert_index < map->size));
    map->freed_count = found_freed_slot ? map->freed_count - 1 : map->freed_count;
    map->elements[insert_index].state = NODE_ALLOCATED;
    map->elements[insert_index].key = key;
    map->elements[insert_index].value = _value;
    ++map->allocated_count;
    log_trace ( "MAP:%s Inserting [%zu] %s key:%p value:%p freed:%zu allocated:%zu size:%zu",
                map->name, insert_index, _value, key, _value, map->freed_count, map->allocated_count, map->size);
    return true;
}

bool insert (map_t *map, void *key, void *_value) {
    resize_if_needed (map);
    return _insert (map, key, _value);
}

void *erase (map_t *map, void *key) {
    size_t index = (HASH_SEED * ((size_t) key)) % map->size;
    map_node *elem_list = map->elements;

    for (size_t itr = 0; itr < map->size; ++itr, index = (index + 1) % map->size) {
        log_debug ( "Iterating [%zu] state:%d %s key:%p value:%p",
                    index, elem_list[index].state, elem_list[index].value ? elem_list[index].value : "NULL", elem_list[index].key, elem_list[index].value);
        if (elem_list[index].state == NODE_UNUSED) {
            break;
        }
        else if (elem_list[index].state == NODE_ALLOCATED) {
            if (elem_list[index].key == key) {
                elem_list[index].state = NODE_FREED;
                elem_list[index].key = NULL;
                --map->allocated_count;
                ++map->freed_count;
                log_trace ( "Erasing [%zu] key:%p value:%p freed:%zu allocated:%zu size:%zu",
                            index, key, elem_list[index].value, map->freed_count, map->allocated_count, map->size);
                return elem_list[index].value;
            }
        }
    }

    log_warn ("Removing :%p from map, key not found, returning NULL", key);
    return NULL;
}

void *find (map_t *map, void *key) {
    size_t index = (HASH_SEED * ((size_t) key)) % map->size;
    map_node *elem_list = map->elements;

    for (size_t itr = 0; itr < map->size; ++itr, index = (index + 1) % map->size) {
        log_debug ( "Iterating [%zu] state:%d %s key:%p value:%p",
                    index, elem_list[itr].state, elem_list[index].value ? elem_list[index].value : "NULL", elem_list[index].key, elem_list[index].value);
        if (elem_list[index].state == NODE_UNUSED) {
            break;
        }
        else if (elem_list[index].state == NODE_ALLOCATED) {
            if (elem_list[index].key == key) {
                log_trace ( "Finding [%zu] key:%p value:%p freed:%zu allocated:%zu size:%zu",
                            index, key, elem_list[index].value, map->freed_count, map->allocated_count, map->size);
                return elem_list[index].value;
            }
        }
    }

    /* returning NULL in case key not found */
    return NULL;
}

bool is_map_empty (map_t *map) {
    return (map->allocated_count == 0);
}

void dump_map (map_t *map) {
    map_node *elem_list = map->elements;
    for (size_t itr = 0; (itr < map->size); ++itr) {
        log_debug ( "Dumping [%zu] state:%d %s key:%p value:%p",
                    itr, elem_list[itr].state, elem_list[itr].value ? elem_list[itr].value : "NULL", elem_list[itr].key, elem_list[itr].value);
    }
}