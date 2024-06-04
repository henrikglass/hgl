
/**
 * LICENSE:
 *
 * MIT License
 *
 * Copyright (c) 2024 Henrik A. Glass
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 * MIT License
 *
 *
 * ABOUT:
 *
 * hgl_io.h implements a handful of functions to make file I/O easy.
 *
 *
 * USAGE:
 *
 * Include hgl_io.h file like this:
 *
 *     #define HGL_IO_IMPLEMENTATION
 *     #include "hgl_io.h"
 *
 * The user of this library may provide their own memory allocator by redefining:
 *
 *     HGL_IO_ALLOC    (default: malloc)
 *     HGL_IO_FREE     (default: free)
 *
 * Normal read & write example:
 *
 *     HglFile file = hgl_io_file_read("myfile.txt");
 *     if (file.data == NULL) {
 *         printf("Error reading file\n");
 *         return 1;
 *     }
 *     file.data[0] = '\0'; // write null terminator to first byte, for some reason.
 *     int err = hgl_io_file_write(&file);
 *     if (err != 0) {
 *         printf("Error writing file\n");
 *     }
 *     hgl_io_file_free(&file);
 *
 * Memory mapped file example:
 *
 *     HglFile file = hgl_io_file_mmap("myfile.txt");
 *     if (file.data == NULL) {
 *         printf("Error mmap:ing file\n");
 *         return 1;
 *     }
 *     file.data[0] = '\0'; // write null terminator to first byte, for some reason.
 *     hgl_io_file_munmap(&file);
 *
 *
 * AUTHOR: Henrik A. Glass
 *
 */

#ifndef HGL_IO_H
#define HGL_IO_H

#include <stdint.h>
#include <stddef.h>

typedef enum
{
    HGL_FILE_MODE_NORMAL,
    HGL_FILE_MODE_MEMORY_MAPPED,
} HglFileMode;

typedef struct
{
    const char *path;
    HglFileMode mode;
    uint8_t *data;
    size_t size;
    size_t it;
} HglFile;

/**
 * Creates a file at `filepath` with the size `size`.
 */
void hgl_io_file_create(const char *filepath, size_t size);

/**
 * Reads the file at `filepath` in binary mode. On error, the HglFile object is
 * returned with `data` set to NULL.
 */
HglFile hgl_io_file_read(const char *filepath);

/**
 * Memory maps (mmap) file at `filepath`. On error, the HglFile object is
 * returned with `data` set to NULL.
 */
HglFile hgl_io_file_mmap(const char *filepath);

/**
 * Writes the data associated with `file` to the underlying file. Returns -1
 * on error, 0 otherwise.
 */
int hgl_io_file_write(HglFile *file);

/**
 * Writes `size` bytes from `data` to the file at `filepath` in binary mode.
 * Returns -1 on error, 0 otherwise.
 */
int hgl_io_file_write_to(const char *filepath, uint8_t *data, size_t size);

/**
 * Appends `size` bytes from `data` to the file at `filepath` in binary mode.
 * Returns -1 on error, 0 otherwise.
 */
int hgl_io_file_append_to(const char *filepath, uint8_t *data, size_t size);

/**
 * Gets the next line in the file `file`. Returns the size of the line in bytes and
 * sets `line` to point to the first character in the line.
 */
int hgl_io_file_get_next_line(HglFile *file, const char **line);

/**
 * Resets the internal iterator of `file`. The next call to, for instance,
 * hgl_io_file_get_next_line after a call to hgl_io_file_reset_iterator will return
 * the first line in the file.
 */
void hgl_io_file_reset_iterator(HglFile *file);

/**
 * Frees the data held by `file`.
 */
void hgl_io_file_free(HglFile *file);

/**
 * Unmaps the memory held by the memory mapped file `file`.
 */
void hgl_io_file_munmap(HglFile *file);

#endif /* HGL_IO_H */

#ifdef HGL_IO_IMPLEMENTATION

#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>

/* CONFIGURABLE: HGL_BUFFERED_CHAN_ALLOC, HGL_BUFFERED_CHAN_FREE */
#if !defined(HGL_IO_ALLOC) && !defined(HGL_IO_FREE)
#  include <stdlib.h>
#  define HGL_IO_ALLOC    malloc
#  define HGL_IO_FREE     free
#endif

void hgl_io_file_create(const char *filepath, size_t size)
{
    FILE *fp = fopen(filepath, "w");
    fseek(fp, size - 1, SEEK_SET);
    fputc('\0', fp);
    fclose(fp);
}

HglFile hgl_io_file_read(const char *filepath)
{
    HglFile file = {
        .path = filepath,
        .mode = HGL_FILE_MODE_NORMAL,
        .data = NULL,
        .size = 0,
        .it   = 0
    };

    /* open file in read binary mode */
    FILE *fp = fopen(filepath, "rb");
    if (fp == NULL) {
        fprintf(stderr, "[hgl_io_file_read] Error: errno=%s\n", strerror(errno));
        goto out;
    }

    /* get file size */
    fseek(fp, 0, SEEK_END);
    ssize_t file_size = ftell(fp);
    rewind(fp);
    if (file_size < 0) {
        fprintf(stderr, "[hgl_io_file_read] Error: errno=%s\n", strerror(errno));
        goto out_close;
    }

    /* allocate memory for data */
    uint8_t *data = HGL_IO_ALLOC(file_size);
    if (data == NULL) {
        fprintf(stderr, "[hgl_io_file_read] Error: Allocation of %zu bytes failed.\n", file_size);
        goto out_close;
    }

    /* read file */
    ssize_t n_read_bytes = fread(data, 1, file_size, fp);
    if (n_read_bytes != file_size) {
        fprintf(stderr, "[hgl_io_file_read] Error: Failed reading file %s\n", filepath);
        HGL_IO_FREE(data);
        goto out_close;
    }

    /* populate `file` before returning */
    file.data = data;
    file.size = (size_t) file_size;

out_close:
    fclose(fp);
out:
    return file;

}

HglFile hgl_io_file_mmap(const char *filepath)
{
    HglFile file = {
        .path = filepath,
        .mode = HGL_FILE_MODE_MEMORY_MAPPED,
        .data = NULL,
        .size = 0,
        .it   = 0
    };

    /* open file in read binary mode */
    int fd = open(filepath, O_RDWR);
    if (fd == -1) {
        fprintf(stderr, "[hgl_io_file_mmap] Error: errno=%s\n", strerror(errno));
        goto out;
    }

    /* get file size */
    struct stat sb;
    fstat(fd, &sb);
    if (fd == -1) {
        fprintf(stderr, "[hgl_io_file_mmap] Error: errno=%s\n", strerror(errno));
        goto out_close;
    }

    /* mmap file */
    uint8_t *data = mmap(NULL,                   /* Let kernel choose page-aligned address */
                         sb.st_size,             /* Length of mapping in bytes */
                         PROT_READ | PROT_WRITE, /* Readable & writable */
                         MAP_SHARED,             /* Shared mapping. Visible to other processes */
                         fd,                     /* File descriptor of file to map */
                         0);                     /* Begin map at offset 0 into file */
    if (data == MAP_FAILED) {
        fprintf(stderr, "[hgl_io_file_mmap] Error: mmap failed with errno=%s\n",
                strerror(errno));
        goto out_close;
    }

    file.data = data;
    file.size = (size_t) sb.st_size;

out_close:
    close(fd);
out:
    return file;
}

int hgl_io_file_write(HglFile *file)
{
    if (file->mode == HGL_FILE_MODE_MEMORY_MAPPED) {
        fprintf(stderr, "[hgl_io_file_write] Error: HglFile is memory mapped.");
        return -1;
    }

    if (file->path == NULL || strlen(file->path) == 0) {
        fprintf(stderr, "[hgl_io_file_write] Error: HglFile has no path or empty path.");
        return -1;
    }

    if (file->data == NULL) {
        fprintf(stderr, "[hgl_io_file_write] Error: HglFile has no associated data.");
        return -1;
    }

    return hgl_io_file_write_to(file->path, file->data, file->size);
}

int hgl_io_file_write_to(const char *filepath, uint8_t *data, size_t size)
{
    /* open file in write binary mode */
    FILE *fp = fopen(filepath, "wb");
    if (fp == NULL) {
        fprintf(stderr, "[hgl_io_file_write_to] Error: errno=%s\n", strerror(errno));
        return -1;
    }

    /* write file data */
    size_t n_written_bytes = fwrite(data, 1, size, fp);
    if (n_written_bytes != size) {
        fprintf(stderr, "[hgl_io_file_write_to] Error: Failed writing to file %s\n", filepath);
        fclose(fp);
        return -1;
    }

    fclose(fp);
    return 0;
}

int hgl_io_file_append_to(const char *filepath, uint8_t *data, size_t size)
{
    /* open file in write binary mode */
    FILE *fp = fopen(filepath, "ab");
    if (fp == NULL) {
        fprintf(stderr, "[hgl_io_file_write_to] Error: errno=%s\n", strerror(errno));
        return -1;
    }

    /* write file data */
    size_t n_written_bytes = fwrite(data, 1, size, fp);
    if (n_written_bytes != size) {
        fprintf(stderr, "[hgl_io_file_write_to] Error: Failed writing to file %s\n", filepath);
        fclose(fp);
        return -1;
    }

    fclose(fp);
    return 0;
}

int hgl_io_file_get_next_line(HglFile *file, const char **line)
{
    if (file->it >= file->size) {
        *line = NULL;
        return 0;
    }

    size_t line_start = file->it;
    while ((file->it < file->size) &&
           (file->data[file->it] != '\n')) {
        file->it++;
    }
    int n_read_bytes = file->it - line_start;
    file->it++;

    *line = (char *) &file->data[line_start];
    return n_read_bytes;

}

void hgl_io_file_reset_iterator(HglFile *file)
{
    file->it = 0;
}

void hgl_io_file_free(HglFile *file)
{
    assert(file->mode == HGL_FILE_MODE_NORMAL);
    HGL_IO_FREE(file->data);
    file->data = NULL;
    file->size = 0;
}

void hgl_io_file_munmap(HglFile *file)
{
    assert(file->mode == HGL_FILE_MODE_MEMORY_MAPPED);
    munmap(file->data, file->size);
    file->data = NULL;
    file->size = 0;
}

#endif
