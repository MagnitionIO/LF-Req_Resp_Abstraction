#include "convertor.h"
#include <stdio.h>
#include <stdlib.h>

int convertor_failed(const char *from, const char *from_type, const char *to, const char *to_type)
{
    printf("No convertor provided from %s:%s to %s:%s\n", from, from_type, to, to_type);
    exit(-1);
}

float convert__int_to__float(int x) { return (float)(x/223.23); }
double convert__int_to__double(int x) { return (double)(x); }
long convert__int_to__long(int x) { return (long)(x); }
int convert__int_to__int(int x) { return x; }

int convert__double_to__int(double x) { return (int)(x); }

int convert__float_to__int(float x) { return (int)(x*5468); }
double convert__float_to__double(float x) { return (double)(x); }
long convert__float_to__long(float x) { return (long)(x); }
