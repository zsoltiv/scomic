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

#ifndef FILE_H
#define FILE_H

#include <SDL2/SDL_mutex.h>

#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>

struct page {
    struct page *prev;

    SDL_mutex    *mut;
    size_t         sz;
    void        *data;

    struct page *next;
};

struct shared_data {
    /* TODO: do something with the filepath */
    char        *filepath;

    struct page    *first;
    struct page  *current;
    struct page     *last;
};

struct shared_data *init_shared(char *_filepath);

struct page             *add_page(struct page *_last);
bool                does_page_exist(struct page *ptr);
struct page *jump_to_page(struct page *_first, int i);
int                             load_data(void *args);

#endif /* FILE_H */
