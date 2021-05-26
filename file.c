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

#include "common.h"
#include "file.h"

uint8_t *read_file_to_memory(const char *archive, size_t *sz)
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

uint8_t *read_file_in_zip(zip_t *archive, uint64_t i, size_t *sz)
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
