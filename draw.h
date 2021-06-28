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

#ifndef DRAW_H
#define DRAW_H

#include <SDL2/SDL.h>

struct sdl_ctx {
    SDL_Window          *win;
    SDL_Surface    *win_surf;

    SDL_Renderer       *rend;

    /* window dimensions */
    int                win_w;
    int                win_h;

    int             offset_x;
    int             offset_y;

    SDL_DisplayMode       dm;

    SDL_Rect            rect;

    /* the drawable page */
    SDL_Surface        *page;
};

struct sdl_ctx *init_sdl();
SDL_Surface *load_page(struct page *_pg);
void draw(struct sdl_ctx *ctx);

#endif /* DRAW_H */
