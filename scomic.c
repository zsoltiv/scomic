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


int main(int argc, char **argv)
{
    if(argc != 3)
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

    printf("first file: %s\n", zip_get_name(za, 0, ZIP_FL_ENC_RAW));

    SDL_Surface **pages = malloc(MAX_IMAGES_LOADED * sizeof(SDL_Surface *));

    SDL_Event e;

    bool run = true;

    int64_t current_page = (argc == 3) ? atoi(argv[2]) : 0;
    int64_t prev_page;
    int64_t low, high;

    get_high_and_low(current_page, num_entries, &low, &high);
    load_pages(pages, za, current_page, low, high, win_surf);

    /* main loop */
    while(run)
    {
        prev_page = current_page;

        /* handle keyboard */
        SDL_PollEvent(&e);

        switch(e.type)
        {
            case SDL_QUIT:
                run = false;
                break;
            case SDL_KEYDOWN:
                switch(e.key.keysym.sym)
                {
                    case SDLK_q:
                        run = false;
                        break;
                    case SDLK_UP:
                        current_page = (current_page <= 0) ? 0 : (current_page - 1);
                        break;
                    case SDLK_DOWN:
                        current_page++;
                        break;
                    default:
                        break;
                }
                break;
            default:
                break;
        }

        /* if the pages have changed, load new pages */
        if(current_page != prev_page) {
            /* TODO: make the loading part more efficient */
            free_pages(pages);
            get_high_and_low(current_page, num_entries, &low, &high);
            load_pages(pages, za, current_page, low, high, win_surf);
        }

        draw(win, win_surf, pages[current_page]);
    }

    zip_error_fini(&error);
    zip_source_close(src);
    zip_source_free(src);
    zip_close(za);

    SDL_FreeSurface(win_surf);
    SDL_DestroyWindow(win);

    IMG_Quit();
    SDL_Quit();
    return 0;
}
