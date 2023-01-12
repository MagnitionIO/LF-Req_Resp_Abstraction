#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "queue.h"
//#include "core/threaded/reactor_threaded.c"

_CALLOC_(q_node);

unsigned int queue_size (queue_t *queue) {
    return queue->size;
}

bool is_empty (queue_t *queue) {
    return queue->size == 0;
}

void init_minima (q_stat *stat) {
    memset (stat, 0, sizeof(q_stat));
    stat->size = ~0;
}

void init_maxima (q_stat *stat) {
    memset (stat, 0, sizeof(q_stat));
}

void update_stats (queue_t *queue, unsigned int size, uint32_t id, uint64_t ts) {
    q_stat *maxima = &queue->maxima;
    q_stat *minima = &queue->minima;

    if (size > maxima->size) {
        maxima->size = size;
        maxima->id = id;
        maxima->ts = ts;
    }

    if (size < minima->size) {
        minima->size = size;
        minima->id = id;
        minima->ts = ts;
    }
}

void update_service_stats (queue_t *queue, int64_t rtt) {
    queue->service_rtt += rtt;
    ++queue->served;
}

void init_queue (queue_t *queue, const char *queue_name) {
    memset (queue, 0, sizeof(queue_t));
    init_minima (&queue->minima);
    init_maxima (&queue->maxima);
    unsigned int ret = snprintf (queue->name, MAX_QUEUE_NAME, "%s", queue_name);
    if (ret >= MAX_QUEUE_NAME) {
        log_error ("Queue name too long");
        queue->name[MAX_QUEUE_NAME - 1] = 0;
    }
}

bool enqueue (queue_t *queue, void *_data) {
    q_node *itr = queue->head;
    uint32_t new_value = *((uint32_t *) _data);
    q_node **to_add = &queue->head;

    if (itr == NULL) {
        goto add_new_entry;
    }

    while (itr != NULL) {
        uint32_t id_ref = *((uint32_t *) itr->data);
        if (id_ref == new_value) {
            log_debug ("Trying to add value:%u, Queue already has value:%u",
                        new_value, id_ref);
            return false;
        }
        to_add = &itr->next;
        itr = itr->next;
    }

add_new_entry:
    *to_add = CALLOC_q_node(1);
    (*to_add)->data = _data;
    ++queue->size;
    return true;
}

bool p_enqueue (queue_t *queue, void *_data) {
    q_node *itr = queue->head;
    uint32_t new_value = *((uint32_t *) _data);
    q_node **to_add = &queue->head;
    int64_t ts = lf_time_logical_elapsed();

    if (itr == NULL) {
        goto add_new_entry;
    }

    while (itr != NULL) {
        uint32_t id_ref = *((uint32_t *) itr->data);
        if (id_ref == new_value) {
            log_debug ("Trying to add value:%u, Queue already has value:%u",
                        new_value, id_ref);
            return false;
        }

        if (id_ref > new_value) {
            break;
        }
        to_add = &itr->next;
        itr = itr->next;
    }

add_new_entry:
    *to_add = CALLOC_q_node(1);
    (*to_add)->data = _data;
    (*to_add)->q_ts = ts;
    (*to_add)->next = itr;
    ++queue->size;
    update_stats (queue, queue->size, new_value, ts);
    return true;
}

void *pop_by_id (queue_t *queue, uint32_t _id) {
    q_node *itr = queue->head;
    q_node **to_pop = &queue->head;
    void *ret = NULL;
    int64_t ts = lf_time_logical_elapsed();
    while (itr != NULL) {
        uint32_t id_ref = *((uint32_t *) itr->data);
        if (id_ref == _id) {
            *to_pop = itr->next;
            ret = itr->data;
            update_service_stats (queue, ts - itr->q_ts);
            free (itr);
            --queue->size;
            update_stats (queue, queue->size, _id, ts);
            return ret;
        }
        to_pop = &itr->next;
        itr = itr->next;
    }

    log_error ("Not found :%u in the list", _id);
    return ret;
}

void *peek_by_id (queue_t *queue, uint32_t _id) {
    q_node *itr = queue->head;
    while (itr != NULL) {
        uint32_t id_ref = *((uint32_t *) itr->data);
        if (id_ref == _id) {
            return itr->data;
        }
        itr = itr->next;
    }

    log_error ("Not found :%u in the list", _id);
    return NULL;
}

void *pop (queue_t *queue) {
    if (is_empty (queue)) {
        log_error ("Popping from Empty Queue");
        return NULL;
    }

    int64_t ts = lf_time_logical_elapsed();
    q_node *ent = queue->head;
    void *ret = ent->data;
    queue->head = ent->next;
    update_service_stats (queue, ts - ent->q_ts);
    free (ent);
    --queue->size;
    uint32_t id = *((uint32_t *) ret);
    update_stats (queue, queue->size, id, lf_time_logical_elapsed());
    
    return ret;
}

void *peek (queue_t *queue) {
    if (is_empty (queue)) {
        log_error ("Peeking from Empty Queue");
        return NULL;
    }

    q_node *ent = queue->head;
    return ent->data;
}

void display (queue_t *queue) {
    q_node *itr = queue->head;
    int counter = 0;
    while (itr != NULL) {
        uint32_t id_ref = *((uint32_t *) itr->data);
        log_debug ("elem[%d]:%u", counter, id_ref);
        itr = itr->next;
        ++counter;
    }
    if (queue->served > 0) {
        float avg_rtt = queue->service_rtt / (queue->served > 0 ? queue->served: 1);
        log_debug ( "%s # SERVICE_STATS # TOTAL_RTT:%lld served:%u AVG_RTT:%f",
                    queue->name, queue->service_rtt, queue->served, avg_rtt);
        log_debug ( "%s # MAXIMA_STATS # max_size:%u ts:%llu id:%u",
                    queue->name, queue->maxima.size, queue->maxima.ts, queue->maxima.id);
        log_debug ( "%s # MINIMA_STATS # min_size:%u ts:%llu id:%u",
                    queue->name, queue->minima.size, queue->minima.ts, queue->minima.id);
    } else {
        log_debug ("UNUSED QUEUE:%s", queue->name);
    }
}

uint32_t get_next_event_id (queue_t *queue) {
    return queue->next_event_id;
}

void inc_next_event_id (queue_t *queue) {
    ++queue->next_event_id;
}

uint32_t get_inc_next_event_id (queue_t *queue) {
    return queue->next_event_id++;
}