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

#include <SDL2/SDL_error.h>
#include <SDL2/SDL_render.h>
#include <SDL2/SDL_rwops.h>
#include <SDL2/SDL_video.h>
#include <SDL2/SDL_image.h>

#include "common.h"
#include "file.h"
#include "draw.h"


SDL_Surface *load_page(struct page *_pg)
{
        SDL_RWops *rw = SDL_RWFromMem(_pg->data, (int) _pg->sz);
        if(!rw) {
            fprintf(stderr, "failed to read image data: %s\n", SDL_GetError());
            return NULL;
        }

        SDL_Surface *img = IMG_Load_RW(rw, 1);
        if(!img) {
            fprintf(stderr, "couldn't open image: %s\n", IMG_GetError());
            return NULL;
        }

        printf("loaded page successfully\n");
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
        fprintf(stderr, "couldn't create texture from surface: %s\n", SDL_GetError());
        return;
    }

    SDL_RenderClear(renderer);

    if(SDL_RenderCopy(renderer, tex, NULL, &rect) < 0) {
        fprintf(stderr, "failed to copy texture onto display: %s\n", SDL_GetError());
        die("failed to render image");
    }

    SDL_RenderPresent(renderer);


    SDL_DestroyTexture(tex);
}

