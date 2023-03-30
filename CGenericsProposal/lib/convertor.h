#ifndef CONVERTOR_H_
#define CONVERTOR_H_

int convertor_failed(const char *from, const char *from_type, const char *to, const char *to_type);

#define Typename(x) _Generic((x), \
    int: "int",                   \
    float: "float",               \
    double: "double",             \
    long: "long",                 \
    char: "char",                 \
    char*: "string",              \
    default: "unknown"            \
)

#define CONVERT(x, y) \
    _Generic((x), \
    int: _Generic((y), \
         float: CALL_CONVERTOR(int, float, x), \
         double: CALL_CONVERTOR(int, double, x), \
         long: CALL_CONVERTOR(int, long, x), \
         default: (__typeof__(y))convertor_failed(#x, Typename(x), #y, Typename(y))), \
    float: _Generic((y), \
           int: CALL_CONVERTOR(float, int, x), \
           double: CALL_CONVERTOR(float, double, x), \
           long: CALL_CONVERTOR(float, long, x), \
           default: (__typeof__(y))convertor_failed(#x, Typename(x), #y, Typename(y))), \
    default: (__typeof__(y))convertor_failed(#x, Typename(x), #y, Typename(y)) \
)



float convert__int_to__float(int x);
double convert__int_to__double(int x);
long convert__int_to__long(int x);
int convert__int_to__int(int x);

int convert__double_to__int(double x);

int convert__float_to__int(float x);
double convert__float_to__double(float x);
long convert__float_to__long(float x);

#endif // CONVERTOR_H_