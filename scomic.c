#include <SDL2/SDL_error.h>
#include <SDL2/SDL_surface.h>
#include <SDL2/SDL_video.h>
#include <zip.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <zipconf.h>

static void die(const char *msg);
static uint8_t *read_file_to_memory(const char *archive, size_t *sz);
static SDL_Surface *load_img(const char *path, SDL_Surface *win_surf);

int main(int argc, char **argv)
{
    if(argc != 2)
        die("usage: scomic [FILE]");

    if(SDL_Init(SDL_INIT_VIDEO) < 0)
        die("failed to initialize SDL2");

    int flags = IMG_INIT_JPG | IMG_INIT_PNG;
    if((IMG_Init(flags) & flags) != flags)
        die("failed to initialize SDL_image");

    size_t sz;
    void *data = read_file_to_memory(argv[1], &sz);

    zip_error_t error;
    zip_error_init(&error);

    zip_source_t *src = zip_source_buffer_create(data, sz, 1, &error);
    if(!src) {
        free(data);
        zip_error_fini(&error);
        die("failed to create source from data");
    }

    zip_t *za = zip_open_from_source(src, 0, &error);
    if(!za) {
        zip_source_free(src);
        zip_error_fini(&error);
        die("failed to create archive from source");
    }

    SDL_DisplayMode dm;
    if(SDL_GetDesktopDisplayMode(0, &dm) != 0) {
        SDL_Log("%s\n", SDL_GetError());
        die("failed to get resolution");
    }

    SDL_Window *win = SDL_CreateWindow("scomic",
        SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
        dm.w, dm.h, SDL_WINDOW_SHOWN);

    int64_t num_entries = zip_get_num_entries(za, 0);
    printf("entries: %ld\n", num_entries);

    zip_error_fini(&error);
    IMG_Quit();
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

    uint8_t *buf = calloc(*sz, sizeof(uint8_t));

    if(fread(buf, sizeof(uint8_t), *sz, fp) < *sz)
        die("failed to read from file");

    fclose(fp);

    return buf;
}

static SDL_Surface *load_img(const char *path, SDL_Surface *win_surf)
{
    SDL_Surface *img = IMG_Load(path);
    if(!img) {
        SDL_Log("couldn't open %s: %s\n", path, SDL_GetError());
        die("failed to load image");
    }

    SDL_Surface *optimized_surf = SDL_ConvertSurface(img, win_surf->format, 0);
    if(!optimized_surf) {
        SDL_Log("%s\n", SDL_GetError());
        die("failed to optimize image");
    }

    SDL_FreeSurface(img);
    
    return optimized_surf;
}
