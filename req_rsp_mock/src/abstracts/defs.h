#ifndef __USER_DEFS_H__
#define __USER_DEFS_H__

#include <stdbool.h>
#include <stdint.h>
#include "queue.h"
#include "translation.h"

typedef struct cache__ctrl__req
{
    uint32_t id;
    uint32_t cache_id;
    uint64_t ts;
    uint64_t lbn;
    uint32_t len;
    uint32_t bitmask;
} cache__ctrl__req;

typedef struct cache__ctrl__interim_rsp
{
    uint32_t id;
    uint32_t cache_id;
    uint64_t ts;
    uint64_t lbn;
    uint32_t len;
} cache__ctrl__interim_rsp;

//void *process_input (void *_connector, void *_input);

typedef struct ctrl__storage__q_ent
{
    uint32_t id;
    uint32_t ctrl_id;
    uint64_t ts;
    uint64_t lbn;
    uint32_t len;
    queue_t *parent_q;
    uint32_t parent_id;
} ctrl__storage__q_ent;

typedef struct ctrl__storage__req
{
    uint32_t id;
    uint32_t ctrl_id;
    uint64_t ts;
    uint64_t lbn;
    uint32_t len;
} ctrl__storage__req;

void *schedule_reference (scheduler_container *scheduler, port_container *outport);
int64_t emulate_delay (scheduler_container *scheduler, port_container *inport, port_container *outport);
int64_t enqueue_task (port_container *inport);
int64_t client_interim_response (port_container *inport, port_container *outport);
int64_t client_response (port_container *inport, port_container *outport);
int64_t process_batch_request (port_container *inport, scheduler_container *scheduler);
void *schedule_disk_request (scheduler_container *scheduler, port_container *outport);
int64_t process_disk_response (port_container *inport, port_container *outport, port_container *other_inport, port_container *other_outport, port_container *other_interim_outport);

#endif