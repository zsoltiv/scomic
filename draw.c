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
}

SDL_Surface *load_page(zip_t *archive, int64_t i)
{
    SDL_Surface *img;

    printf("page: %ld\n", i);

    size_t sz;
    uint8_t *buf = read_file_in_zip(archive, i, &sz);

    img = load_img(buf, sz);
        
    free(buf);

    return img;
}

void draw(SDL_Renderer *renderer, SDL_Window *win_ptr, SDL_Surface *page)
{
    int w, h;
    SDL_GetWindowSize(win_ptr, &w, &h);
    SDL_Rect rect;
    rect.x = 0;
    rect.y = 0;
    rect.w = (w == 0) ? 100 : w;
    rect.h = (h == 0) ? 300 : h;

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
