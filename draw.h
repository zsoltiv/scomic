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

SDL_Surface *load_page(struct page *_pg);
void draw(SDL_Renderer *renderer, SDL_Window *win_ptr, SDL_Surface *page);

#endif /* DRAW_H */
