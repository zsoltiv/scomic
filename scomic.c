/*  This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU Affero General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Affero General Public License for more details.

    You should have received a copy of the GNU Affero General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>. */

#include <SDL2/SDL_error.h>
#include <SDL2/SDL_events.h>
#include <SDL2/SDL_rwops.h>
#include <SDL2/SDL_surface.h>
#include <SDL2/SDL_video.h>
#include <zip.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdint.h>

/* the maximum number of images stored as SDL_Surfaces at the same time */
#define MAX_IMAGES_LOADED 11

static void die(const char *msg);
static uint8_t *read_file_to_memory(const char *archive, size_t *sz);
static SDL_Surface *load_img(const uint8_t *buf, int size, SDL_Surface *win_surf);
static uint8_t *read_file_in_zip(zip_t *archive, uint64_t i, size_t *sz);
static void load_pages(SDL_Surface **images, zip_t *archive, int64_t i,
    int64_t entries, SDL_Surface *win_surf);

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

    while(true)
    {
        SDL_PollEvent(&e);

        if(e.type == SDL_QUIT)
            break;

        load_pages(pages, za, atoi(argv[2]), num_entries, win_surf);
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

static void die(const char *msg)
{
    fprintf(stderr, "ERROR: %s\n", msg);
    exit(EXIT_FAILURE);
}

static uint8_t *read_file_to_memory(const char *archive, size_t *sz)
{
    FILE *fp = fopen(archive, "rb");
    if(!fp)
        die("failed to open archive");

    fseek(fp, 0, SEEK_END);
    *sz = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    uint8_t *buf = malloc(*sz * sizeof(uint8_t));

    if(fread(buf, sizeof(uint8_t), *sz, fp) < *sz)
        die("failed to read from file");

    fclose(fp);

    return buf;
}

static SDL_Surface *load_img(const uint8_t *buf, int size, SDL_Surface *win_surf)
{
    SDL_RWops *rw = SDL_RWFromMem(buf, size);
    SDL_Surface *img = IMG_Load_RW(rw, 1);
    if(!img) {
        SDL_Log("couldn't open image: %s\n", SDL_GetError());
        return NULL;
    }

    SDL_Surface *optimized_surf = SDL_ConvertSurface(img, win_surf->format, 0);
    if(!optimized_surf) {
        SDL_Log("%s\n", SDL_GetError());
        die("failed to optimize image");
    }

    SDL_FreeSurface(img);
    
    return optimized_surf;
}

static uint8_t *read_file_in_zip(zip_t *archive, uint64_t i, size_t *sz)
{
    zip_stat_t stats;
    zip_stat_init(&stats);

    /* get size of file in archive */
    zip_stat_index(archive, i, ZIP_FL_ENC_RAW, &stats);
    if((stats.valid & ZIP_STAT_SIZE) == ZIP_STAT_SIZE)
        printf("file size: %lu\n", stats.size);
    else
        die("failed to get file size");

    zip_file_t *fp = zip_fopen_index(archive, i, ZIP_FL_UNCHANGED);
    if(!fp) {
        fprintf(stderr, "%s\n", zip_file_strerror(fp));
        die("failed to open file in archive");
    }
    
    *sz = stats.size;

    uint8_t *buffer = calloc(*sz, sizeof(uint8_t));

    if(zip_fread(fp, buffer, *sz) == -1)
        die("failed to read from file in archive");

    zip_fclose(fp);
    return buffer;
}

/* we load the previous 5, the current and the next 5 pages */
static void load_pages(SDL_Surface **images, zip_t *archive,
    int64_t i, int64_t entries, SDL_Surface *win_surf)
{
    /* the indices of the furthest pages */
    int64_t low, high;
    
    /* this is the number of loaded pages before and after the current page */
    const int64_t half_of_extra_pages = (MAX_IMAGES_LOADED - 1) / 2;

    /* set the low and high indices */
    if(i - half_of_extra_pages < 0)
        low = 0;
    else
        low = i - half_of_extra_pages;

    if(i + half_of_extra_pages >= entries)
        high = entries - 1;
    else
        high = i + half_of_extra_pages;

    for(int index = 0; index < MAX_IMAGES_LOADED; index++)
    {
        printf("index: %d\n", index);
        size_t sz;
        uint8_t *buf = read_file_in_zip(archive, low + index, &sz);
        images[index] = load_img(buf, sz, win_surf);
        
        free(buf);

    }
}
