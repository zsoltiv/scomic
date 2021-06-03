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
#include <SDL2/SDL_error.h>
#include <SDL2/SDL_render.h>
#include <SDL2/SDL_video.h>

SDL_Surface *load_img(const uint8_t *buf, int size)
{
    SDL_RWops *rw = SDL_RWFromMem((void *) buf, size);
    if(!rw) {
        printf("%s\n", SDL_GetError());
        die("SDL_RWFromMem() failed");
    }

    SDL_Surface *img = IMG_Load_RW(rw, 1);
    if(!img) {
        SDL_Log("couldn't open image: %s\n", SDL_GetError());
        return NULL;
    }

    return img;

    /*SDL_Surface *optimized_surf = SDL_ConvertSurface(img, win_surf->format, 0);
    printf("optimized img\n");
    if(!optimized_surf) {
        SDL_Log("%s\n", SDL_GetError());
        die("failed to optimize image");
    }

    SDL_FreeSurface(img);
    
    return optimized_surf;*/
}

void get_high_and_low(int64_t i, int64_t entries, int64_t *low, int64_t *high)
{
    /* this is the number of loaded pages before and after the current page */
    const int64_t half_of_extra_pages = (MAX_IMAGES_LOADED - 1) / 2;

    *low = (i - half_of_extra_pages < 0) ? 0 : (i - half_of_extra_pages);
    *high = (i + half_of_extra_pages >= entries) ? (entries - 1) : (i + half_of_extra_pages);
}

SDL_Surface *load_page(zip_t *archive, int64_t i)
{
    SDL_Surface *img;

    printf("index: %ld\npage: %ld\n", i, i);
    size_t sz;
    uint8_t *buf = read_file_in_zip(archive, i, &sz);

    img = load_img(buf, sz);
        
    free(buf);

    return img;
}

/* we load the previous 5, the current and the next 5 pages */
void load_pages(SDL_Surface **images, zip_t *archive,
    int64_t i, int64_t low, int64_t high, SDL_Surface *win_surf)
{
    for(int index = 0; index < MAX_IMAGES_LOADED; index++)
        images[index] = load_page(archive, i);
}

void free_pages(SDL_Surface **_pages)
{
    for(int i = 0; i < MAX_IMAGES_LOADED; i++)
        SDL_FreeSurface(_pages[i]);
}

void draw(SDL_Renderer *renderer, SDL_Window *win_ptr, SDL_Surface *page)
{
    int w, h;
    SDL_GetWindowSize(win_ptr, &w, &h);
    SDL_Rect rect;
    rect.x = 1;
    rect.y = 1;
    rect.w = (w == 0) ? 100 : w;
    rect.h = (h == 0) ? 300 : h;

    if(page) {
        SDL_Texture *tex = SDL_CreateTextureFromSurface(renderer, page);
        if(!tex) {
            printf("%s\n", SDL_GetError());
            die("failed to create texture from surface");
        }

        SDL_RenderClear(renderer);

        if(SDL_RenderCopy(renderer, tex, NULL, &rect) < 0) {
            printf("%s\n", SDL_GetError());
            die("failed to render image");
        }

        SDL_RenderPresent(renderer);

        SDL_DestroyTexture(tex);
    }

}
