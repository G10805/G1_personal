#pragma once
#include <unistd.h>
namespace udp_protocol {
// If you want to add some get_x function, remember not to use function like
// "sizeof()" Just use a hard code number. for example, if put int, we put 32
// bits.
char get_byte(const char* buff, int* offset);
short get_short(const char* buff, int* offset);
int get_int(const char* buff, int* offset);
long long get_long(const char* buff, int* offset);
float get_float(const char* buff, int* offset);
double get_double(const char* buff, int* offset);
const char* get_string(const char* buff, int* offset);
int get_binary(const char* buff, int* offset, char* output);
void put_byte(char* buff, int* offset, const char input);
void put_short(char* buff, int* offset, const short input);
void put_int(char* buff, int* offset, const int input);
void put_long(char* buff, int* offset, const long long input);
void put_float(char* buff, int* offset, const float input);
void put_double(char* buff, int* offset, const double input);
void put_string(char* buff, int* offset, const char* input);
void put_binary(char* buff, int* offset, const char* input, const int len);
}  // namespace udp_protocol
