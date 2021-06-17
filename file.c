/*  This file is part of scomic.

    scomic is free software: you can redistribute it and/or modify
    it under the terms of the GNU Affero General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    scomic is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Affero General Public License for more details.

    You should have received a copy of the GNU Affero General Public License
    along with scomic.  If not, see <https://www.gnu.org/licenses/>. */

#include <SDL2/SDL_mutex.h>
#include <archive.h>
#include <archive_entry.h>

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "common.h"
#include "file.h"

#define LIBARCHIVE_BUF_SZ 8192


static const char *supported_extensions[] = {
    "jpg",
    "jpeg",
    "png",
    "bmp"
};


static char *get_extension(const char *filepath);
static bool is_file_img(const char *filepath);

struct page *add_page(struct page *_last)
{
   struct page *new = malloc(sizeof(struct page));

   if(_last)
       _last->next = new;

   new->prev   =                        _last;
   new->next   =                         NULL;
   new->mut    =            SDL_CreateMutex();
   new->data   =                         NULL;
   new->sz     =                            0;

   return new;
}

int load_data(void *args)
{
    struct shared_data *_shared = args;

    struct archive *ar = archive_read_new();
    if(!ar)
        return EXIT_FAILURE;

    archive_read_support_filter_all(ar);
    archive_read_support_format_all(ar);

    int r = archive_read_open_filename(ar, _shared->filepath, LIBARCHIVE_BUF_SZ);
    if(r != ARCHIVE_OK) {
        printf("libarchive error: %s\n", archive_error_string(ar));
        archive_read_free(ar);
        return EXIT_FAILURE;
    }

    struct archive_entry *e;

    struct page *last = _shared->first;
    struct page *prev =           last;

    while((r = archive_read_next_header(ar, &e)) != ARCHIVE_EOF) {
        if(r < ARCHIVE_WARN) {
            printf("libarchive error: %s\n", archive_error_string(ar));
            archive_read_close(ar);
            archive_read_free(ar);
            return EXIT_FAILURE;
        }

        /* we don't read anything unless the entry
         * is an image file supported by SDL2_image */
        char *ext = get_extension(archive_entry_pathname(e));
        if(archive_entry_filetype(e) != AE_IFREG
            || !is_file_img(ext))
            continue;

        int64_t offset;

        last->sz = archive_entry_size(e);
        last->data = malloc(last->sz);
        if((r = archive_read_data(ar, last->data,
            last->sz)) != ARCHIVE_EOF) {
            if(r < ARCHIVE_WARN) {
                printf("archive_read_data() error: %s\n", archive_error_string(ar));
                break;
            }
        }

        prev = last;
        last = add_page(prev);
    }


    archive_read_close(ar);
    archive_read_free(ar);

    return EXIT_SUCCESS;
}

static char *get_extension(const char *filepath)
{
    const char delim[2] = ".";
    char *ptr;
    char *str = strtok(filepath, delim);

    while(str) {
        ptr = str;
        str = strtok(NULL, delim);
    }

    return ptr;
}

static bool is_file_img(const char *filepath)
{
    char *ext = get_extension(filepath);

    if(!ext)
        return NULL;

    for(int i = 0; i < sizeof(supported_extensions) / sizeof(char *); i++) {
        if(strcmp(ext, supported_extensions[i]) == 0)
            return true;
    }

    return false;
}
