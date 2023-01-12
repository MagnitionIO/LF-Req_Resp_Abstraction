#include "defs.h"
#include "utils.h"
#include "include/ctarget/set.h"

static unsigned int rand_seed = 10;
static uint64_t _lbn = 0;
void *schedule_reference (scheduler_container *scheduler, port_container *outport) {
    connector_t *connector = (connector_t *) scheduler->parent_connector;

    log_debug ( "(%lld, %u) physical_time:%lld "
                "CONNECTOR:%s Port:%s Processing...",
                lf_time_logical_elapsed(), lf_tag().microstep, lf_time_physical_elapsed(),
                connector->name, scheduler->port_str
    );

    uint32_t id =  get_inc_next_event_id (&outport->queue);
    unsigned int _len = (rand_r(&rand_seed) % 10) + 1;
    cache__ctrl__req *req = _CALLOC (1, sizeof (cache__ctrl__req));
    {
        req->cache_id = outport->connector_id;
        req->id = id;
        req->ts = lf_time_logical_elapsed(),
        req->lbn = _lbn;
        req->len = _len;
        for (int i = 0; i < _len; ++i) {
            req->bitmask |= (1 << i);
        }
    }
    log_debug ( "(%lld, %u) physical_time:%lld "
                "CONNECTOR:%s Port:%s Scheduling fetch id:%u cache_id:%d lbn:%llu len:%u ts:%llu bm:%u",
                lf_time_logical_elapsed(), lf_tag().microstep, lf_time_physical_elapsed(),
                connector->name, outport->port_str, req->id, req->cache_id, req->lbn, req->len, req->ts, req->bitmask
    );

    bool ret = PRIORITY_ENQUEUE (&outport->queue, req);
    log_debug ( "(%lld, %u) physical_time:%lld "
                "CONNECTOR:%s queue:%s id:%u:%s",
                lf_time_logical_elapsed(), lf_tag().microstep, lf_time_physical_elapsed(),
                connector->name, outport->queue.name, req->id, ret ? "SUCCESS" : "FAILURE"
    );
    memcpy (outport->data, req, sizeof (cache__ctrl__req));
    _lf_set_present(outport->is_present);
    return req;
}

int64_t enqueue_task (port_container *inport) {
    connector_t *connector = (connector_t *) inport->parent_connector;

    log_debug ( "(%lld, %u) physical_time:%lld "
                "CONNECTOR:%s Port:%s Processing...",
                lf_time_logical_elapsed(), lf_tag().microstep, lf_time_physical_elapsed(),
                connector->name, inport->port_str
    );

    assert (inport->data_size == sizeof (ctrl__storage__req));
    ctrl__storage__req *req = _CALLOC (inport->data_elements, sizeof (ctrl__storage__req));
    memcpy (req, inport->data, inport->data_size * inport->data_elements);
    bool ret = PRIORITY_ENQUEUE (&inport->queue, req);
    log_debug ( "(%lld, %u) physical_time:%lld "
                "CONNECTOR:%s queue:%s id:%u:%s",
                lf_time_logical_elapsed(), lf_tag().microstep, lf_time_physical_elapsed(),
                connector->name, inport->queue.name, req->id, ret ? "SUCCESS" : "FAILURE"
    );
    return req->id;
}

int64_t emulate_delay (scheduler_container *scheduler, port_container *inport, port_container *outport) {
    connector_t *connector = (connector_t *) scheduler->parent_connector;

    uint32_t *event_id = (uint32_t *) scheduler->data;
    log_debug ( "(%lld, %u) physical_time:%lld "
                "CONNECTOR:%s Port:%s Processing Disk event:%u...",
                lf_time_logical_elapsed(), lf_tag().microstep, lf_time_physical_elapsed(),
                connector->name, scheduler->port_str, *event_id
    );

    ctrl__storage__req *req = pop_by_id (&inport->queue, *event_id);
    if (req == NULL) {
        log_error ( "(%lld, %u) physical_time:%lld "
                    "CONNECTOR:%s Failed Disk Request id:%u not found",
                    lf_time_logical_elapsed(), lf_tag().microstep, lf_time_physical_elapsed(),
                    connector->name, *event_id
        );
        return -1;
    }
    log_debug ( "(%lld, %u) physical_time:%lld "
                "CONNECTOR:%s Finished Disk Request id:%u",
                lf_time_logical_elapsed(), lf_tag().microstep, lf_time_physical_elapsed(),
                connector->name, req->id
    );
    _FREE (req);

    memcpy (outport->data, event_id, sizeof (uint32_t));
    _lf_set_present(outport->is_present);
    return *event_id;
}

int64_t client_interim_response (port_container *inport, port_container *outport) {
    connector_t *connector = (connector_t *) inport->parent_connector;

    cache__ctrl__interim_rsp *rsp = (cache__ctrl__interim_rsp *) inport->data;
    log_debug ( "(%lld, %u) physical_time:%lld "
                "CONNECTOR:%s Port:%s Processing Interim Response event:%u lbn:%llu len:%u ts:%llu...",
                lf_time_logical_elapsed(), lf_tag().microstep, lf_time_physical_elapsed(),
                connector->name, inport->port_str, rsp->id, rsp->lbn, rsp->len, rsp->ts
    );
    cache__ctrl__req *req = peek_by_id (&outport->queue, rsp->id);
    log_debug ( "(%lld, %u) physical_time:%lld "
                "CONNECTOR:%s Interim Response req:%p id:%u lbn:%llu len:%u ts:%llu bitmask:%u",
                lf_time_logical_elapsed(), lf_tag().microstep, lf_time_physical_elapsed(),
                connector->name, req, req->id, req->lbn, req->len, req->ts, req->bitmask
    );
    if (req == NULL) {
        log_error ( "(%lld, %u) physical_time:%lld "
                    "CONNECTOR:%s Failed Interim Response id:%u not found",
                    lf_time_logical_elapsed(), lf_tag().microstep, lf_time_physical_elapsed(),
                    connector->name, rsp->id
        );
        return -1;
    }
    uint32_t bit = 1 << (rsp->lbn - req->lbn);
    req->bitmask &= ~bit;

    log_debug ( "(%lld, %u) physical_time:%lld "
                "CONNECTOR:%s Filling block id:%u lbn:%llu len:%u ts:%llu bit:%u bitmask:%u",
                lf_time_logical_elapsed(), lf_tag().microstep, lf_time_physical_elapsed(),
                connector->name, rsp->id, rsp->lbn, rsp->len, rsp->ts, bit, req->bitmask
    );

    if (req->bitmask == 0) {
        log_debug ( "(%lld, %u) physical_time:%lld "
                    "CONNECTOR:%s Waiting to cleanup request id:%u lbn:%llu len:%u ts:%llu bit:%u bitmask:%u",
                    lf_time_logical_elapsed(), lf_tag().microstep, lf_time_physical_elapsed(),
                    connector->name, rsp->id, rsp->lbn, rsp->len, rsp->ts, bit, req->bitmask
        );
    }
    return rsp->id;
}

int64_t client_response (port_container *inport, port_container *outport) {
    connector_t *connector = (connector_t *) inport->parent_connector;

    uint32_t *event_id = (uint32_t *) inport->data;
    log_debug ( "(%lld, %u) physical_time:%lld "
                "CONNECTOR:%s Port:%s Processing Response event:%u...",
                lf_time_logical_elapsed(), lf_tag().microstep, lf_time_physical_elapsed(),
                connector->name, inport->port_str, *event_id
    );
    // @TODO pop by id should be passed with current logical time, for average response calculations
    cache__ctrl__req *req = pop_by_id (&outport->queue, *event_id);
    if (req == NULL) {
        log_error ( "(%lld, %u) physical_time:%lld "
                    "CONNECTOR:%s Failed Response id:%u not found",
                    lf_time_logical_elapsed(), lf_tag().microstep, lf_time_physical_elapsed(),
                    connector->name, *event_id
        );
        return -1;
    }

    if (req->bitmask > 0) {
        log_error ( "(%lld, %u) physical_time:%lld "
                    "CONNECTOR:%s BITMASK not ZERO id:%u lbn:%llu len:%u ts:%llu bitmask:%u",
                    lf_time_logical_elapsed(), lf_tag().microstep, lf_time_physical_elapsed(),
                    connector->name, req->id, req->lbn, req->len, req->ts, req->bitmask
        );
    }

    log_debug ( "(%lld, %u) physical_time:%lld "
                "CONNECTOR:%s FINISHED request id:%u lbn:%llu len:%u ts:%llu bitmask:%u",
                lf_time_logical_elapsed(), lf_tag().microstep, lf_time_physical_elapsed(),
                connector->name, req->id, req->lbn, req->len, req->ts, req->bitmask
    );
    _FREE (req);
    return *event_id;
}

int64_t process_batch_request (port_container *inport, scheduler_container *scheduler) {
    connector_t *connector = (connector_t *) inport->parent_connector;
    cache__ctrl__req *req = _CALLOC (inport->data_elements, sizeof(cache__ctrl__req));
    memcpy (req, inport->data, inport->data_elements * inport->data_size);
    log_debug ("(%lld, %u) physical_time:%lld "
                "CONNECTOR:%s Fetch Request id:%u cache_id[%d] lbn:%llu len:%u ts:%llu",
                lf_time_logical_elapsed(), lf_tag().microstep, lf_time_physical_elapsed(),
                connector->name, req->id, req->cache_id, req->lbn, req->len, req->ts
    );

    bool ret = PRIORITY_ENQUEUE (&inport->queue, req);
    log_debug ( "(%lld, %u) physical_time:%lld "
                "CONNECTOR:%s p_enqueue id:%u:%s",
                lf_time_logical_elapsed(), lf_tag().microstep, lf_time_physical_elapsed(),
                connector->name, req->id, ret ? "SUCCESS" : "FAILURE"
    );

    bool storage_fetch_trigger = is_empty (&scheduler->queue);
    unsigned int bm = req->bitmask;
    for (int i = 0; bm > 0; ++i) {
        uint32_t bit = 1;
        if (bm & bit) {
            uint32_t id =  get_inc_next_event_id (&scheduler->queue);
            ctrl__storage__q_ent *out_q_ent = _CALLOC (1, sizeof(ctrl__storage__q_ent));
            out_q_ent->id = id;
            out_q_ent->ctrl_id = connector->connector_id;
            out_q_ent->ts = lf_time_logical_elapsed();
            out_q_ent->lbn = req->lbn + i;
            out_q_ent->len = 1;
            out_q_ent->parent_id = req->id;
            ret = PRIORITY_ENQUEUE (&scheduler->queue, out_q_ent);
            log_debug ( "(%lld, %u) physical_time:%lld "
                        "CONNECTOR:%s Scheduling Request Queued p_enqueue id:%u parent_id:%u lbn:%llu bm:%u status:%s",
                        lf_time_logical_elapsed(), lf_tag().microstep, lf_time_physical_elapsed(),
                        connector->name, out_q_ent->id, out_q_ent->parent_id, out_q_ent->lbn, bm, ret ? "SUCCESS" : "FAILURE"
            );
        }
        bm >>= 1;
    }
    return req->id;
}

void *schedule_disk_request (scheduler_container *scheduler, port_container *outport) {
    connector_t *connector = (connector_t *) scheduler->parent_connector;
    bool ret = false;
    if (is_empty (&scheduler->queue) == false) {
        ctrl__storage__q_ent *q_ent = pop (&scheduler->queue);
        ret = PRIORITY_ENQUEUE (&outport->queue, q_ent);
        ctrl__storage__req req = {  .id = q_ent->id,
                                    .ctrl_id = q_ent->ctrl_id,
                                    .ts = q_ent->ts,
                                    .lbn = q_ent->lbn,
                                    .len = q_ent->len };
        log_debug ( "(%lld, %u) physical_time:%lld "
                    "CONNECTOR:%s Sending Request to disk id:%u parent_id[%d] lbn:%llu len:%u ts:%llu",
                    lf_time_logical_elapsed(), lf_tag().microstep, lf_time_physical_elapsed(),
                    connector->name, req.id, q_ent->parent_id, req.lbn, req.len, req.ts
        );
        memcpy (outport->data, &req, sizeof (ctrl__storage__req));
        _lf_set_present(outport->is_present);
        return q_ent;
    }
    return NULL;
}

int64_t process_disk_response (port_container *inport, port_container *outport, port_container *other_inport, port_container *other_outport, port_container *other_interim_outport) {
    connector_t *connector = (connector_t *) inport->parent_connector;
    int64_t ret = -1;
    if (is_empty (&outport->queue) == false) {
        uint32_t *event_id = (uint32_t *) inport->data;
        ctrl__storage__q_ent *q_ent = pop_by_id (&outport->queue, *event_id);
        cache__ctrl__req *req = peek_by_id (&other_inport->queue, q_ent->parent_id);
        uint32_t bit = 1 << (q_ent->lbn - req->lbn);
        cache__ctrl__interim_rsp interim_rsp = {.id = req->id,
                                                .cache_id = req->cache_id,
                                                .ts = lf_time_logical_elapsed(),
                                                .lbn = q_ent->lbn,
                                                .len = q_ent->len };
        memcpy (other_interim_outport->data, &interim_rsp, sizeof (cache__ctrl__interim_rsp));
        _lf_set_present(other_interim_outport->is_present);

        req->bitmask &= ~bit;

        log_debug ( "(%lld, %u) physical_time:%lld "
                    "CONNECTOR:%s Checking Response from disk id:%u parent_id[%d] lbn:%llu len:%u ts:%llu bit:%u bitmask:%u",
                    lf_time_logical_elapsed(), lf_tag().microstep, lf_time_physical_elapsed(),
                    connector->name, q_ent->id, q_ent->parent_id, q_ent->lbn, q_ent->len, q_ent->ts, bit, req->bitmask
        );

        if (req->bitmask == 0) {
            log_debug ( "(%lld, %u) physical_time:%lld "
                        "CONNECTOR:%s Sending cleanup trig for id:%u parent_id[%d] lbn:%llu len:%u ts:%llu bit:%u bitmask:%u",
                        lf_time_logical_elapsed(), lf_tag().microstep, lf_time_physical_elapsed(),
                        connector->name, q_ent->id, q_ent->parent_id, q_ent->lbn, q_ent->len, q_ent->ts, bit, req->bitmask
            );
            memcpy (other_outport->data, &req->id, sizeof (uint32_t));
            _lf_set_present(other_outport->is_present);

            req = pop_by_id (&other_inport->queue, req->id);
            _FREE (req);
        }
        _FREE (q_ent);
        ret = *event_id;
    }
    return ret;
}