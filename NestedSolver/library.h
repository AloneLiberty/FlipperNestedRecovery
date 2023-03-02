#ifndef MIFARE_LIB_LIBRARY_H
#define MIFARE_LIB_LIBRARY_H
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

char* run_nested(uint32_t uid, uint32_t nt0, uint32_t ks0, uint32_t nt1, uint32_t ks1);

char* run_full_nested(uint32_t uid, uint32_t nt0, uint32_t ks0, uint32_t par0, uint32_t nt1, uint32_t ks1, uint32_t par1, int from, int to);
#endif //MIFARE_LIB_LIBRARY_H
