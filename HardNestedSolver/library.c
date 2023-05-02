#include "cmdhfmfhard.h"
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>

char* run_hardnested(uint32_t uid, char* path) {
    uint64_t foundkey = 0;
    if (mfnestedhard(0, 0, NULL, 0, 0, NULL, false, false, false, &foundkey, NULL, uid, path) == 1) {
        char* keystr = malloc(14);
        snprintf(keystr, 14, "%012" PRIx64 ";", foundkey);
        return keystr;
    } else {
        return calloc(1, 1);
    }
}
