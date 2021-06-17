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
#include <SDL2/SDL_thread.h>
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

static void handle_input(bool *run, struct shared_data *_shared);

static void dump_shared_data(struct shared_data *_shared);
static int get_existing_page_count(struct shared_data *_shared);
static void dump_page(struct page *_pg);

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

    /* init second thread */
    struct shared_data *shared = malloc(sizeof(struct shared_data));
    shared->filepath =        argv[1];
    shared->first    = add_page(NULL);
    shared->current  =  shared->first;
    shared->last     =  shared->first;

    if(load_data((void *) shared) == EXIT_FAILURE)
        die("load_data returned EXIT_FAILURE\n");

    SDL_Surface *page = NULL;

    SDL_Renderer *rend = SDL_CreateRenderer(win, -1, SDL_RENDERER_ACCELERATED);
    if(!rend) {
        fprintf(stderr, "%s\n", SDL_GetError());
        die("failed to create renderer");
    }
    SDL_SetRenderDrawColor(rend, 0, 0, 0, 255);

    bool run = true;

    printf("start main loop\n");

    /* main loop */
    while(run)
    {
        handle_input(&run, shared);

        page = load_page(shared->current);

        if(page) {
            draw(rend, win, page);

            //SDL_FreeSurface(page);
        }

        /* debug stuff */
        //dump_page(shared->current);
        //dump_shared_data(shared);
    }

    //SDL_FreeSurface(win_surf);
    //SDL_DestroyRenderer(rend);
    //SDL_DestroyWindow(win);

    //IMG_Quit();
    //SDL_Quit();

    return 0;
}

static void handle_input(bool *run, struct shared_data *_shared)
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
                    if(_shared->current->prev) {
                        _shared->current = _shared->current->prev;
                        printf("decrement page\n");
                    }
                    break;
                case SDLK_DOWN:
                    if(_shared->current->next) {
                        _shared->current = _shared->current->next;
                        printf("increment page\n");
                    }
                    break;
                case SDLK_k:
                    if(_shared->current->prev) {
                        _shared->current = _shared->current->prev;
                        printf("decrement page\n");
                    }
                    break;
                case SDLK_j:
                    if(_shared->current->next) {
                        _shared->current = _shared->current->next;
                        printf("increment page\n");
                    }
                    break;
                default:
                    break;
            }
            break;
        default:
            break;
    }
}

static void dump_shared_data(struct shared_data *_shared)
{
    printf("------shared data------\nfile: %s\naddress of current page: %p\n",
        _shared->filepath, _shared->current);
}

static void dump_page(struct page *_pg)
{
    printf("page pointer: %p\ndata pointer: %p\nnext: %p\nprev: %p\n size: %ld\n",
        _pg, _pg->data, _pg->next, _pg->prev, _pg->sz);
}
