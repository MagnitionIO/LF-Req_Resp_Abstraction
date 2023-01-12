#ifndef __TRANSLATION_H__
#define __TRANSLATION_H__

/*
 * includes
 *
 */

#include <string.h>
#include <stdbool.h>
#include "utils.h"
#include "map.h"
#include "queue.h"
#include "include/ctarget/set.h"
#include "include/core/reactor.h"

#define MAX_NAME_LEN  100
typedef struct port_container port_container;
typedef struct scheduler_container scheduler_container;
typedef struct connector_t connector_t;


typedef enum {
    OUTPUT_TYPE_1 = 0,
    INTERIM_OUTPUT_TYPE,
    REQUEST_INPUT_TYPE_1,
    REQUEST_INPUT_TYPE_2,
    RESPONSE_INPUT_TYPE_1,
    RESPONSE_INPUT_TYPE_2,
    RESPONSE_INTERIM_INPUT_TYPE,
} port_type;

typedef union {
    int64_t (*process_request_1) (port_container *inport);
    int64_t (*process_response_1) (port_container *inport, port_container *outport);
    int64_t (*process_request_2) (port_container *inport, scheduler_container *scheduler);
    int64_t (*process_response_2) (port_container *inport, port_container *outport, port_container *other_inport, port_container *other_outport, port_container *other_interim_outport);
} port_function;

typedef struct port_container {
    uint32_t connector_id;
    port_type type;
    queue_t queue;
    void *port;
    void *data;
    size_t data_size;
    uint32_t data_elements;
    bool *is_present;
    bool inbound;
    void *parent_connector;
    char *port_str;
    port_function processor;
} port_container;

typedef enum {
    REQUEST_SCHEDULER_TYPE_1 = 0,
    REQUEST_SCHEDULER_TYPE_2,
    RESPONSE_SCHEDULER_TYPE_1,
} scheduler_type;

typedef union {
    void* (*schedule_output_1) (scheduler_container* scheduler, port_container* outport);
    int64_t (*schedule_output_2) (scheduler_container *scheduler, port_container *inport, port_container *outport);
} scheduler_function;

typedef struct scheduler_container {
    uint32_t connector_id;
    scheduler_type type;
    queue_t queue;
    void *scheduler;
    void *data;
    size_t data_size;
    uint32_t data_elements;
    port_container *linked_inport;
    port_container *linked_outport;
    bool logical;
    void *parent_connector;
    char *port_str;
    scheduler_function processor;
} scheduler_container;

typedef struct connector_t {
    uint32_t connector_id;
    uint32_t reactor_id;
    char name[MAX_NAME_LEN];
    uint32_t tag;
    uint32_t last_len;
    scheduler_container scheduler_inst;
    port_container input;
    port_container output;
    port_container interim_input;
    port_container interim_output;
    connector_t *linked_connector;
} connector_t;

#define init_connector(connector, _reactor_id, _name, _connector_id) \
do { \
    memset (connector, 0, sizeof (connector_t)); \
    connector->connector_id = _connector_id; \
    connector->reactor_id = _reactor_id; \
    unsigned int ret = snprintf (connector->name, MAX_NAME_LEN, "%s:%u:%u", _name, connector->reactor_id, connector->connector_id); \
    if (ret >= MAX_NAME_LEN) { \
        log_error ("Connector name too long"); \
        connector->name[MAX_NAME_LEN - 1] = 0; \
    } \
} while(0)

#define link_connectors(connector_1, connector_2) \
do { \
    connector_1->linked_connector = connector_2; \
    connector_2->linked_connector = connector_1; \
} while(0)

#define setup_scheduler(connector, _type, schd, _data, _sz, _elems, process_schd) \
do { \
    char name[MAX_QUEUE_NAME]; \
    memset (&connector->scheduler_inst, 0, sizeof (scheduler_container)); \
    connector->scheduler_inst.connector_id = connector->connector_id; \
    connector->scheduler_inst.scheduler = schd; \
    connector->scheduler_inst.type = _type; \
    connector->scheduler_inst.data = _data; \
    connector->scheduler_inst.data_size = _sz; \
    connector->scheduler_inst.data_elements = _elems; \
    connector->scheduler_inst.logical = true; \
    connector->scheduler_inst.parent_connector = connector; \
    connector->scheduler_inst.port_str = "SCHEDULER"; \
    unsigned int ret = snprintf (name, MAX_QUEUE_NAME, "%s-%s", "SCHEDULER_QUEUE", connector->name); \
    if (ret >= MAX_QUEUE_NAME) { \
        log_error ("Queue name too long"); \
        name[MAX_QUEUE_NAME - 1] = 0; \
    } \
    init_queue (&connector->scheduler_inst.queue, name); \
    switch (_type) { \
    case REQUEST_SCHEDULER_TYPE_1: \
    case REQUEST_SCHEDULER_TYPE_2: \
        connector->scheduler_inst.processor.schedule_output_1 = process_schd; \
        connector->scheduler_inst.linked_outport = &connector->output; \
        break; \
    case RESPONSE_SCHEDULER_TYPE_1: \
        connector->scheduler_inst.processor.schedule_output_2 = process_schd; \
        connector->scheduler_inst.linked_outport = &connector->output; \
        connector->scheduler_inst.linked_inport = &connector->input; \
        break; \
    default: \
        log_error ("Undefined port type:%d", _type); \
        break; \
    } \
} while(0)

#define setup_req_scheduler_type_1(connector, schd, _data, _sz, _elems, process_schd) setup_scheduler(connector, REQUEST_SCHEDULER_TYPE_1, schd, _data, _sz, _elems, process_schd)
#define setup_rsp_scheduler_type_1(connector, schd, _data, _sz, _elems, process_schd) setup_scheduler(connector, RESPONSE_SCHEDULER_TYPE_1, schd, _data, _sz, _elems, process_schd)
#define setup_req_scheduler_type_2(connector, schd, _data, _sz, _elems, process_schd) setup_scheduler(connector, REQUEST_SCHEDULER_TYPE_2, schd, _data, _sz, _elems, process_schd)

#define setup_req_scheduler_type_2_(connector, schd, process_schd) setup_req_scheduler_type_2(connector, schd, NULL, 0, 0, process_schd)

#define setup_port(connector, port_container, _direction, _str, _type, _port, _data, _sz, _elems, _is_present, process_port) \
do { \
    char name[MAX_QUEUE_NAME]; \
    memset (&port_container, 0, sizeof (port_container)); \
    port_container.connector_id = connector->connector_id; \
    port_container.port = _port; \
    port_container.type = _type; \
    port_container.data = _data; \
    port_container.data_size = _sz; \
    port_container.data_elements = _elems; \
    port_container.is_present = _is_present; \
    port_container.inbound = _direction; \
    port_container.parent_connector = connector; \
    port_container.port_str = _str; \
    unsigned int ret = snprintf (name, MAX_QUEUE_NAME, "%s%s-%s", _str, "_QUEUE", connector->name); \
    if (ret >= MAX_QUEUE_NAME) { \
        log_error ("Queue name too long"); \
        name[MAX_QUEUE_NAME - 1] = 0; \
    } \
    init_queue (&port_container.queue, name); \
    if (process_port != NULL) { \
        switch (_type) { \
        case OUTPUT_TYPE_1: \
        case REQUEST_INPUT_TYPE_1: \
            port_container.processor.process_request_1 = process_port; \
            break; \
        case REQUEST_INPUT_TYPE_2: \
            port_container.processor.process_request_2 = process_port; \
            break; \
        case RESPONSE_INPUT_TYPE_1: \
        case RESPONSE_INTERIM_INPUT_TYPE: \
            port_container.processor.process_response_1 = process_port; \
            break; \
        case RESPONSE_INPUT_TYPE_2: \
            port_container.processor.process_response_2 = process_port; \
            break; \
        default: \
            log_error ("Undefined port type:%d", _type); \
            break; \
        } \
    } \
} while(0)

#define setup_type_1_output(connector, _port, _data, _sz, _elems, _is_present, process_port) setup_port(connector, connector->output, false, "OUTPUT_PORT", OUTPUT_TYPE_1, _port, _data, _sz, _elems, _is_present, process_port)
#define setup_interim_output(connector, _port, _data, _sz, _elems, _is_present, process_port) setup_port(connector, connector->interim_output, false, "INTERIM_OUTPUT_PORT", INTERIM_OUTPUT_TYPE, _port, _data, _sz, _elems, _is_present, process_port)
#define setup_req_input_type_1(connector, _port, _data, _sz, _elems, _is_present, process_port) setup_port(connector, connector->input, true, "INPUT_PORT", REQUEST_INPUT_TYPE_1, _port, _data, _sz, _elems, _is_present, process_port)
#define setup_req_input_type_2(connector, _port, _data, _sz, _elems, _is_present, process_port) setup_port(connector, connector->input, true, "INPUT_PORT", REQUEST_INPUT_TYPE_2, _port, _data, _sz, _elems, _is_present, process_port)
#define setup_rsp_input_type_1(connector, _port, _data, _sz, _elems, _is_present, process_port) setup_port(connector, connector->input, true, "INPUT_PORT", RESPONSE_INPUT_TYPE_1, _port, _data, _sz, _elems, _is_present, process_port)
#define setup_rsp_input_type_2(connector, _port, _data, _sz, _elems, _is_present, process_port) setup_port(connector, connector->input, true, "INPUT_PORT", RESPONSE_INPUT_TYPE_2, _port, _data, _sz, _elems, _is_present, process_port)
#define setup_rsp_interim_input_type(connector, _port, _data, _sz, _elems, _is_present, process_port) setup_port(connector, connector->interim_input, true, "INTERIM_INPUT_PORT", RESPONSE_INTERIM_INPUT_TYPE, _port, _data, _sz, _elems, _is_present, process_port)

#define setup_interim_output_(connector, _port, _elems, process_port) setup_interim_output(connector, _port, &_port->value, sizeof(_port->value), _elems, &_port->is_present, process_port)
#define setup_type_1_output_(connector, _port, _elems, process_port) setup_type_1_output(connector, _port, &_port->value, sizeof(_port->value), _elems, &_port->is_present, process_port)
#define setup_req_input_type_2_(connector, _port, _elems, process_port) setup_req_input_type_2(connector, _port, &_port->value, sizeof(_port->value), _elems, &_port->is_present, process_port)
#define setup_rsp_input_type_2_(connector, _port, _elems, process_port) setup_rsp_input_type_2(connector, _port, &_port->value, sizeof(_port->value), _elems, &_port->is_present, process_port)

#define connector_process_scheduler(map, port) \
({ \
    bool ret = false; \
    connector_t *connector = find (map, port); \
    if (connector != NULL) { \
        scheduler_container *sch_inst = &connector->scheduler_inst; \
        switch (sch_inst->type) { \
        case REQUEST_SCHEDULER_TYPE_1: \
            if (sch_inst->processor.schedule_output_1) { \
                void *req = sch_inst->processor.schedule_output_1 (sch_inst, sch_inst->linked_outport); \
                ret = (req != NULL) ? true : false; \
                log_debug ("%s Req:%p RET:%s", connector->name, req, ret ? "TRUE" : "FALSE"); \
            } \
            break; \
        case REQUEST_SCHEDULER_TYPE_2: \
            if (sch_inst->processor.schedule_output_1) { \
                void *req = sch_inst->processor.schedule_output_1 (sch_inst, sch_inst->linked_outport); \
                ret = (req != NULL) ? true : false; \
                log_debug ("%s Req:%p RET:%s", connector->name, req, ret ? "TRUE" : "FALSE"); \
                if (is_empty (&sch_inst->queue) == false) { \
                    lf_schedule (sch_inst->scheduler, 0); \
                } \
            } \
            break; \
        case RESPONSE_SCHEDULER_TYPE_1: \
            if (sch_inst->processor.schedule_output_2) { \
                int64_t req = sch_inst->processor.schedule_output_2 (sch_inst, sch_inst->linked_inport, sch_inst->linked_outport); \
                ret = (req >= 0) ? true : false; \
                log_debug ("%s Req:%lld RET:%s", connector->name, req, ret ? "TRUE" : "FALSE"); \
            } \
            break; \
        default: \
            log_error ("Undefined port type:%d", sch_inst->type); \
            break; \
        } \
    } else { \
        log_error ("Failed to find connector against logical action:%p", port); \
    } \
    ret; \
})

#define connector_process_input(map, _port) \
({ \
    int64_t ret = -1; \
    connector_t *connector = find (map, _port); \
    if (connector != NULL) { \
        port_container *input_inst = (connector->input.port == _port) ? &connector->input : &connector->interim_input; \
        switch (input_inst->type) { \
        case REQUEST_INPUT_TYPE_1: \
            if (input_inst->processor.process_request_1) { \
                int64_t req = input_inst->processor.process_request_1 (input_inst); \
                ret = (req >= 0) ? req : -2; \
                log_debug ("%s Req:%lld RET:%lld", connector->name, req, ret); \
            } \
            break; \
        case REQUEST_INPUT_TYPE_2: \
            if (input_inst->processor.process_request_2) { \
                connector_t *other_connector = connector->linked_connector; \
                scheduler_container *other_sch_inst = &other_connector->scheduler_inst; \
                bool storage_fetch_trigger = is_empty (&other_sch_inst->queue); \
                int64_t req = input_inst->processor.process_request_2 (input_inst, other_sch_inst); \
                ret = (req >= 0) ? req : -2; \
                log_debug ("%s Req:%lld RET:%lld", connector->name, req, ret); \
                if (storage_fetch_trigger) { \
                    lf_schedule (other_sch_inst->scheduler, 0); \
                } \
            } \
            break; \
        case RESPONSE_INTERIM_INPUT_TYPE: \
        case RESPONSE_INPUT_TYPE_1: \
            if (input_inst->processor.process_response_1) { \
                int64_t req = input_inst->processor.process_response_1 (input_inst, &connector->output); \
                ret = (req >= 0) ? req : -2; \
                log_debug ("%s Req:%lld RET:%lld", connector->name, req, ret); \
            } \
            break; \
        case RESPONSE_INPUT_TYPE_2: \
            if (input_inst->processor.process_response_2) { \
                connector_t *other_connector = connector->linked_connector; \
                int64_t req = input_inst->processor.process_response_2 (input_inst, &connector->output, &other_connector->input, &other_connector->output, &other_connector->interim_output); \
                ret = (req >= 0) ? req : -2; \
                log_debug ("%s Req:%lld RET:%lld", connector->name, req, ret); \
            } \
            break; \
        default: \
            log_error ("Undefined port type:%d", input_inst->type); \
            break; \
        } \
    } else { \
        log_error ("Failed to find connector against logical action:%p", _port); \
    } \
    ret; \
})

#define connector_schedule_next(map, port, interval) \
do { \
    connector_t *connector = find (map, port); \
    if (connector != NULL) { \
        lf_schedule (port, interval); \
        log_debug ( "%s CONNECTOR[%d] Next Schedule at time:%lld", connector->name, connector->connector_id, interval); \
    } \
} while(0)

#define connector_schedule_next_copy(map, port, interval, _data) \
do { \
    connector_t *connector = find (map, port); \
    if (connector != NULL) { \
        scheduler_container *sch_inst = &connector->scheduler_inst; \
        lf_schedule_copy (port, interval, _data, sch_inst->data_elements); \
        log_debug ( "CONNECTOR:%s Next Schedule at time:%lld len:%u", connector->name, interval, sch_inst->data_elements); \
    } \
} while(0)

#define connector_service_stats(connector) \
do { \
    log_debug ("CONNECTOR:%s SCHEDULER_STATS", connector.name); \
    display (&connector.scheduler_inst.queue); \
    log_debug ("CONNECTOR:%s INPUT_Q_STATS", connector.name); \
    display (&connector.input.queue); \
    log_debug ("CONNECTOR:%s OUTPUT_Q_STATS", connector.name); \
    display (&connector.output.queue); \
    log_debug ("CONNECTOR:%s INTERIM_INPUT_Q_STATS", connector.name); \
    display (&connector.interim_input.queue); \
    log_debug ("CONNECTOR:%s INTERIM_OUTPUT_Q_STATS", connector.name); \
    display (&connector.interim_output.queue); \
} while(0)

#endif