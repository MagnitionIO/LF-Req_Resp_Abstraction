#ifndef _c_H
#define _c_H
#ifndef TOP_LEVEL_PREAMBLE_453055967_H
#define TOP_LEVEL_PREAMBLE_453055967_H
/*Correspondence: Range: [(6, 4), (6, 47)) -> Range: [(0, 0), (0, 43)) (verbatim=true; src=/Users/khubaibumer/git/LF_Collaboration/tracing/src/C.lf)*/#define TRACE_STR_REACTOR_C "RELAY_POINT_C"
#endif
#ifdef __cplusplus
extern "C" {
#endif
#include "../include/api/api.h"
#include "../include/core/reactor.h"
#ifdef __cplusplus
}
#endif
typedef struct c_self_t{
    self_base_t base; // This field is only to be used by the runtime, not the user.
    int end[0]; // placeholder; MSVC does not compile empty structs
} c_self_t;
typedef struct {
    token_type_t type;
    lf_token_t* token;
    size_t length;
    bool is_present;
    lf_port_internal_t _base;
    int value;
    #ifdef FEDERATED
    #ifdef FEDERATED_DECENTRALIZED
    tag_t intended_tag;
    #endif
    interval_t physical_time_of_arrival;
    #endif
} c_in_t;
typedef struct {
    token_type_t type;
    lf_token_t* token;
    size_t length;
    bool is_present;
    lf_port_internal_t _base;
    int value;
    #ifdef FEDERATED
    #ifdef FEDERATED_DECENTRALIZED
    tag_t intended_tag;
    #endif
    interval_t physical_time_of_arrival;
    #endif
} c_out_t;
#endif
