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
#include <SDL2/SDL_events.h>
#include <SDL2/SDL_render.h>
#include <SDL2/SDL_rwops.h>
#include <SDL2/SDL_surface.h>
#include <SDL2/SDL_video.h>
#include <SDL2/SDL_keyboard.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdint.h>

#include "common.h"
#include "file.h"
#include "draw.h"

static void handle_input(bool *run, int64_t *prev_page, int64_t *current_page, int64_t num_entries);

int main(int argc, char **argv)
{
    if(argc < 2)
        die("usage: scomic [FILE]");

    /* initialize SDL2 and SDL2_image */
    if(SDL_Init(SDL_INIT_VIDEO) < 0)
        die("failed to initialize SDL2");

    int flags = IMG_INIT_JPG | IMG_INIT_PNG;
    if((IMG_Init(flags) & flags) != flags)
        die("failed to initialize SDL_image");

    /* create gui */
    SDL_DisplayMode dm;
    if(SDL_GetDesktopDisplayMode(0, &dm) != 0) {
        SDL_Log("%s\n", SDL_GetError());
        die("failed to get resolution");
    }

    SDL_Window *win = SDL_CreateWindow("scomic",
        SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
        dm.w / 3, dm.h - 100, SDL_WINDOW_SHOWN);

    SDL_Surface *win_surf = SDL_GetWindowSurface(win);

    /* read cbz archive */
    size_t sz;
    void *data = read_file_to_memory(argv[1], &sz);

    /* initialize libzip and open the cbz in memory */
    zip_error_t error;
    zip_error_init(&error);

    zip_source_t *src = zip_source_buffer_create(data, sz, 1, &error);
    if(!src) {
        free(data);
        zip_error_fini(&error);
        die("failed to create source from data");
    }

    zip_t *za = zip_open_from_source(src, ZIP_RDONLY, &error);
    if(!za) {
        zip_source_free(src);
        zip_error_fini(&error);
        die("failed to create archive from source");
    }

    int64_t num_entries = zip_get_num_entries(za, 0);
    printf("entries: %ld\n", num_entries);

    SDL_Surface *page;

    SDL_Renderer *rend = SDL_CreateRenderer(win, -1, SDL_RENDERER_ACCELERATED);
    if(!rend) {
        printf("%s\n", SDL_GetError());
        die("failed to create renderer");
    }
    SDL_SetRenderDrawColor(rend, 0, 0, 0, 255);


    bool run = true;

    int64_t current_page = (argc == 3) ? atoi(argv[2]) : 1;
    int64_t prev_page, low, high;

    page = load_page(za, current_page);

    /* main loop */
    while(run)
    {
        /* handle keyboard */
        handle_input(&run, &prev_page, &current_page, num_entries);

        /* if the pages have changed, load new pages */
        if(current_page != prev_page) {
            SDL_FreeSurface(page);
            page = load_page(za, current_page);
        }

        draw(rend, win, page);
    }

    //zip_close(za);
    //zip_source_close(src);
    //zip_source_free(src);
    //zip_error_fini(&error);

    //SDL_FreeSurface(win_surf);
    //SDL_DestroyRenderer(rend);
    //SDL_DestroyWindow(win);

    //IMG_Quit();
    //SDL_Quit();
    return 0;
}

static void handle_input(bool *run, int64_t *prev_page, int64_t *current_page, int64_t num_entries)
{
    SDL_Event e;
    SDL_PollEvent(&e);

    switch(e.type)
    {
        case SDL_QUIT:
            *run = false;
            break;
        case SDL_KEYDOWN:
            switch(e.key.keysym.sym)
            {
                case SDLK_q:
                    *run = false;
                    break;
                case SDLK_UP:
                    *prev_page = *current_page;
                    *current_page = (*current_page <= 1) ? *current_page : (*current_page - 1);
                    break;
                case SDLK_DOWN:
                    *prev_page = *current_page;
                    *current_page = (*current_page + 1 > num_entries) ? *current_page : *current_page + 1;
                    break;
                case SDLK_k:
                    *prev_page = *current_page;
                    *current_page = (*current_page <= 1) ? *current_page : (*current_page - 1);
                    break;
                case SDLK_j:
                    *prev_page = *current_page;
                    *current_page = (*current_page + 1 > num_entries) ? *current_page : *current_page + 1;
                    break;
                default:
                    break;
            }
            break;
        default:
            break;
    }
}
