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

#include <SDL2/SDL_error.h>
#include <SDL2/SDL_events.h>
#include <SDL2/SDL_rwops.h>
#include <SDL2/SDL_surface.h>
#include <SDL2/SDL_video.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

#include <zip.h>

/* the maximum number of images stored as SDL_Surfaces at the same time */
#define MAX_IMAGES_LOADED 1

SDL_Surface *load_img(const uint8_t *buf, int size);
SDL_Surface *load_page(zip_t *archive, int64_t i);
void draw(SDL_Renderer *renderer, SDL_Window *win_ptr, SDL_Surface *page);

#endif /* DRAW_H */
