#ifndef ___QUEUE_H___
#define ___QUEUE_H___

#include <stdbool.h>
#include <stdint.h>
#include "utils.h"

#define MAX_QUEUE_NAME  100

typedef struct q_stat {
    unsigned int size;
    uint32_t id;
    uint64_t ts;
} q_stat;

typedef struct q_node {
    void *data;
    int64_t q_ts;
    struct q_node *next;
} q_node;

typedef struct queue_t {
    q_node *head;
    unsigned int size;
    q_stat minima;
    q_stat maxima;
    int64_t service_rtt;
    uint32_t served;
    char name[MAX_QUEUE_NAME];
    uint32_t next_event_id;
} queue_t;

int64_t lf_time_logical_elapsed(void);
void init_minima (q_stat *stat);
void init_maxima (q_stat *stat);
void init_queue (queue_t *queue, const char *queue_name);
bool enqueue (queue_t *queue, void *_data);
bool p_enqueue (queue_t *queue, void *_data);
void *pop_by_id (queue_t *queue, uint32_t _id);
void *peek_by_id (queue_t *queue, uint32_t _id);
void *pop (queue_t *queue);
void *peek (queue_t *queue);
void display (queue_t *queue);
unsigned int queue_size (queue_t *queue);
bool is_empty (queue_t *queue);
void update_stats (queue_t *queue, unsigned int size, uint32_t id, uint64_t ts);
uint32_t get_next_event_id (queue_t *queue);
void inc_next_event_id (queue_t *queue);
uint32_t get_inc_next_event_id (queue_t *queue);

#define PRIORITY_ENQUEUE(queue, data) \
({ \
    bool ret = false; \
    if ((&data->id != (uint32_t *) data) || (data->id != *((uint32_t *) data))) { \
        log_error ("Trying to add data to the queue, first element is not the ID"); \
    } \
    else { \
        ret = p_enqueue (queue, data); \
        log_trace ("%s Adding entry:%u ptr:%p to queue status:%d", ((queue_t *)queue)->name, data->id, data, ret); \
    } \
    ret; \
})
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

#define _CALLOC_(TYPE) \
    static inline TYPE *CALLOC_##TYPE(size_t len) { \
        void *allocation = calloc (len, sizeof (TYPE)); \
        log_trace ("_CALLOC_: ptr:%p", allocation); \
        return ((TYPE *) allocation); \
    }

#endif