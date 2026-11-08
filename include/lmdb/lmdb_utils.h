#pragma once

#include <stdio.h>
#include <stdlib.h>
#include "lmdb.h"

/* Abort on any LMDB error. For tools, tests, and startup code. */
#define LMDB_CHECK(call)                                                   \
    do {                                                                   \
        int rc_ = (call);                                                  \
        if (rc_ != MDB_SUCCESS) {                                          \
            fprintf(stderr, "%s:%d: %s failed: %s\n",                      \
                    __FILE__, __LINE__, #call, mdb_strerror(rc_));         \
            exit(EXIT_FAILURE);                                            \
        }                                                                  \
    } while (0)
