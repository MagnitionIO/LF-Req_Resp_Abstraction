#ifndef _main_main_H
#define _main_main_H
#ifndef TOP_LEVEL_PREAMBLE_133685853_H
#define TOP_LEVEL_PREAMBLE_133685853_H
/*Correspondence: Range: [(6, 4), (6, 47)) -> Range: [(0, 0), (0, 43)) (verbatim=true; src=/Users/khubaibumer/git/LF_Collaboration/tracing/src/C.lf)*/#define TRACE_STR_REACTOR_C "RELAY_POINT_C"
/*Correspondence: Range: [(6, 4), (6, 53)) -> Range: [(0, 0), (0, 49)) (verbatim=true; src=/Users/khubaibumer/git/LF_Collaboration/tracing/src/emitter.lf)*/#define TRACE_STR_REACTOR_E "RELAY_POINT_Emitter"
/*Correspondence: Range: [(6, 4), (6, 47)) -> Range: [(0, 0), (0, 43)) (verbatim=true; src=/Users/khubaibumer/git/LF_Collaboration/tracing/src/B.lf)*/#define TRACE_STR_REACTOR_B "RELAY_POINT_B"
/*Correspondence: Range: [(6, 4), (6, 54)) -> Range: [(0, 0), (0, 50)) (verbatim=true; src=/Users/khubaibumer/git/LF_Collaboration/tracing/src/receiver.lf)*/#define TRACE_STR_REACTOR_R "RELAY_POINT_Receiver"
/*Correspondence: Range: [(6, 4), (6, 47)) -> Range: [(0, 0), (0, 43)) (verbatim=true; src=/Users/khubaibumer/git/LF_Collaboration/tracing/src/A.lf)*/#define TRACE_STR_REACTOR_A "RELAY_POINT_A"
#endif
#ifdef __cplusplus
extern "C" {
#endif
#include "../include/api/api.h"
#include "../include/core/reactor.h"
#ifdef __cplusplus
}
#endif
typedef struct main_self_t{
    self_base_t base; // This field is only to be used by the runtime, not the user.
    int end[0]; // placeholder; MSVC does not compile empty structs
} main_self_t;
typedef struct {
    token_type_t type;
    lf_token_t* token;
    size_t length;
    bool is_present;
    lf_port_internal_t _base;
    int value;

} emitter_out_t;
typedef struct {
    token_type_t type;
    lf_token_t* token;
    size_t length;
    bool is_present;
    lf_port_internal_t _base;
    int value;

} a_in_t;
typedef struct {
    token_type_t type;
    lf_token_t* token;
    size_t length;
    bool is_present;
    lf_port_internal_t _base;
    int value;

} a_out_t;
typedef struct {
    token_type_t type;
    lf_token_t* token;
    size_t length;
    bool is_present;
    lf_port_internal_t _base;
    int value;

} b_in_t;
typedef struct {
    token_type_t type;
    lf_token_t* token;
    size_t length;
    bool is_present;
    lf_port_internal_t _base;
    int value;

} b_out_t;
typedef struct {
    token_type_t type;
    lf_token_t* token;
    size_t length;
    bool is_present;
    lf_port_internal_t _base;
    int value;

} c_in_t;
typedef struct {
    token_type_t type;
    lf_token_t* token;
    size_t length;
    bool is_present;
    lf_port_internal_t _base;
    int value;

} c_out_t;
typedef struct {
    token_type_t type;
    lf_token_t* token;
    size_t length;
    bool is_present;
    lf_port_internal_t _base;
    int value;

} reciever_in_t;
#endif
