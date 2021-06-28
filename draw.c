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

struct sdl_ctx *init_sdl()
{
    /* init SDL2 and SDL2_image */
    if(SDL_Init(SDL_INIT_VIDEO) < 0)
        die("failed to initialize SDL2");

    int flags = IMG_INIT_JPG | IMG_INIT_PNG;
    if((IMG_Init(flags) & flags) != flags)
        die("failed to initialize SDL_image");

    struct sdl_ctx *ctx = malloc(sizeof(struct sdl_ctx));

    ctx->page = NULL;

    if(SDL_GetDesktopDisplayMode(0, &ctx->dm) != 0) {
        SDL_Log("%s\n", SDL_GetError());
        die("failed to get resolution");
    }

    /* create window */
    ctx->win = SDL_CreateWindow("scomic",
        SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
        ctx->dm.w / 3, ctx->dm.h - 100, SDL_WINDOW_SHOWN);

    ctx->win_surf = SDL_GetWindowSurface(ctx->win);

    ctx->rend = SDL_CreateRenderer(ctx->win, -1, SDL_RENDERER_ACCELERATED);
    if(!ctx->rend) {
        fprintf(stderr, "%s\n", SDL_GetError());
        die("failed to create renderer");
    }

    SDL_GetWindowSize(ctx->win, &ctx->win_w, &ctx->win_h);

    /* create rectangle to render to */
    ctx->rect.x = 0;
    ctx->rect.y = 0;

    ctx->rect.w = (ctx->win_w == 0) ? 100 : ctx->win_w;
    ctx->rect.h = (ctx->win_h == 0) ? 300 : ctx->win_h;

    SDL_SetRenderDrawColor(ctx->rend, 0, 0, 0, 255);

    return ctx;
}

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

        return img;
}

void draw(struct sdl_ctx *ctx)
{
    SDL_Texture *tex = SDL_CreateTextureFromSurface(ctx->rend, ctx->page);
    if(!tex) {
        fprintf(stderr, "couldn't create texture from surface: %s\n", SDL_GetError());
        return;
    }

    SDL_RenderClear(ctx->rend);

    if(SDL_RenderCopy(ctx->rend, tex, NULL, &ctx->rect) < 0) {
        fprintf(stderr, "failed to copy texture onto display: %s\n", SDL_GetError());
        die("failed to render image");
    }

    SDL_RenderPresent(ctx->rend);

    SDL_DestroyTexture(tex);
}
