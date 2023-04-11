#ifndef MIFARE_LIB_LIBRARY_H
#define MIFARE_LIB_LIBRARY_H
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

typedef struct {
    union {
        struct Crypto1State *slhead;
        uint64_t *keyhead;
    } head;
    union {
        struct Crypto1State *sltail;
        uint64_t *keytail;
    } tail;
    uint32_t len;
    uint32_t uid;
    uint32_t nt_enc;
    uint32_t ks1;
} StateList_t;

typedef struct {
    uint32_t uid;
    uint32_t nt0;
    uint32_t ks0;
    uint32_t nt1;
    uint32_t ks1;
    char *keys;
    bool free;
} InfoList_t;

char* run_nested(uint32_t uid, uint32_t nt0, uint32_t ks0, uint32_t nt1, uint32_t ks1);

char* run_full_nested(uint32_t uid, uint32_t nt0, uint32_t ks0, uint32_t par0, uint32_t nt1, uint32_t ks1, uint32_t par1, int from, int to, bool progress);

#endif //MIFARE_LIB_LIBRARY_H
