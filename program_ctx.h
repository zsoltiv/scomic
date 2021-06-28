#ifndef PROGRAM_CTX_H
#define PROGRAM_CTX_H

#include <stdbool.h>

#include "file.h"
#include "draw.h"

struct program_ctx {
    /* data shared between the two threads */
    struct shared_data   *shared;
    struct sdl_ctx     *draw_ctx;

    bool              run;
    /* is set to true whenever `current` changes */
    bool has_page_changed;
};

#endif /* PROGRAM_CTX_H */
