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

#include "common.h"
#include "file.h"
#include "draw.h"

SDL_Surface *load_img(const uint8_t *buf, int size, SDL_Surface *win_surf)
{
    SDL_RWops *rw = SDL_RWFromMem((void *) buf, size);
    SDL_Surface *img = IMG_Load_RW(rw, 1);

    if(!img) {
        SDL_Log("couldn't open image: %s\n", SDL_GetError());
        return NULL;
    }

    SDL_Surface *optimized_surf = SDL_ConvertSurface(img, win_surf->format, 0);
    if(!optimized_surf) {
        SDL_Log("%s\n", SDL_GetError());
        die("failed to optimize image");
    }

    SDL_FreeSurface(img);
    
    return optimized_surf;
}

void get_high_and_low(int64_t i, int64_t entries, int64_t *low, int64_t *high)
{
    /* this is the number of loaded pages before and after the current page */
    const int64_t half_of_extra_pages = (MAX_IMAGES_LOADED - 1) / 2;

    *low = (i - half_of_extra_pages < 0) ? 0 : (i - half_of_extra_pages);
    *high = (i + half_of_extra_pages >= entries) ? (entries - 1) : (i + half_of_extra_pages);
}

/* we load the previous 5, the current and the next 5 pages */
void load_pages(SDL_Surface **images, zip_t *archive,
    int64_t i, int64_t low, int64_t high, SDL_Surface *win_surf)
{
    for(int index = 0; index < MAX_IMAGES_LOADED; index++)
    {
        printf("index: %d\npage: %ld\n", index, low + index);
        size_t sz;
        uint8_t *buf = read_file_in_zip(archive, low + index, &sz);
        images[index] = load_img(buf, sz, win_surf);
        
        free(buf);
    }
}

void free_pages(SDL_Surface **_pages)
{
    for(int i = 0; i < MAX_IMAGES_LOADED; i++)
    {
        SDL_FreeSurface(_pages[i]);
    }
}

void draw(SDL_Window *win, SDL_Surface *screen_surf, SDL_Surface *page)
{
    SDL_BlitSurface(page, NULL, screen_surf, NULL);

    SDL_UpdateWindowSurface(win);
}
