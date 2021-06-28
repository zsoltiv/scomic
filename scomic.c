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
#include "program_ctx.h"

static void handle_input(struct program_ctx *ctx);

int main(int argc, char **argv)
{
    if(argc < 2)
        die("usage: scomic [FILE]");

    struct program_ctx *prog_ctx = malloc(sizeof(struct program_ctx));

    prog_ctx->draw_ctx         =           init_sdl();
    prog_ctx->shared           = init_shared(argv[1]);
    prog_ctx->run              =                 true;
    prog_ctx->has_page_changed =                false;

    /* init second thread */
    if(load_data((void *) prog_ctx->shared) == EXIT_FAILURE)
        die("load_data returned EXIT_FAILURE\n");

    SDL_Log("Loaded data\n");

    /* main loop */
    while(prog_ctx->run) {
        handle_input(prog_ctx);

        /* avoid redrawing every iteration */
        if(prog_ctx->has_page_changed) {
            SDL_Log("page changed\n");
            
            SDL_FreeSurface(prog_ctx->draw_ctx->page);

            prog_ctx->draw_ctx->page = NULL;
            prog_ctx->has_page_changed = false;
        }

        if(!prog_ctx->draw_ctx->page) {
            SDL_Log("redrawing\n");
            prog_ctx->draw_ctx->page = load_page(prog_ctx->shared->current);

            draw(prog_ctx->draw_ctx);
        }
    }

    //SDL_FreeSurface(win_surf);
    //SDL_DestroyRenderer(rend);
    //SDL_DestroyWindow(win);

    //IMG_Quit();
    //SDL_Quit();

    return 0;
}

static void handle_input(struct program_ctx *ctx)
{
    SDL_Event e;
    SDL_PollEvent(&e);

    switch(e.type)
    {
        case SDL_QUIT:
            ctx->run = false;
            break;
        case SDL_KEYDOWN:
            switch(e.key.keysym.sym)
            {
                case SDLK_q:
                    ctx->run = false;
                    break;
                case SDLK_UP:
                    if(ctx->shared->current->prev) {
                        ctx->shared->current = ctx->shared->current->prev;
                        ctx->has_page_changed = true;
                    }
                    break;
                case SDLK_DOWN:
                    if(ctx->shared->current->next) {
                        ctx->shared->current = ctx->shared->current->next;
                        ctx->has_page_changed = true;
                    }
                    break;
                case SDLK_k:
                    if(ctx->shared->current->prev) {
                        ctx->shared->current = ctx->shared->current->prev;
                        ctx->has_page_changed = true;
                    }
                    break;
                case SDLK_j:
                    if(ctx->shared->current->next) {
                        ctx->shared->current = ctx->shared->current->next;
                        ctx->has_page_changed = true;
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
