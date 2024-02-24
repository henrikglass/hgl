/**
 * LICENSE:
 *
 * MIT License
 *
 * Copyright (c) 2023 Henrik A. Glass
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
 * hgl_hotload.h implements a simple to use hot-reloading library.
 *
 *
 * USAGE:
 *
 * Include hgl_hotload.h file like this:
 *
 *     #define HGL_HOTLOAD_IMPLEMENTATION
 *     #include "hgl_hotload.h"
 *
 * HGL_HOTLOAD_IMPLEMENTATION must only be defined once, in a single compilation unit. 
 * Below is a complete listing of the API (see function prototype comments for more 
 * details):
 *
 *     void hgl_hotload_final(void);
 *     int hgl_hotload_init(const char *lib_paths[], int flags);
 *     int hgl_hotload_add_symbol(void **ptr_to_symbol_handle, 
 *                                const char *lib_path,
 *                                const char *symbol_name);
 *     int hgl_hotload_force_reload_all(void);
 *     int hgl_hotload_reload_lib(const char *lib_path);
 *
 * hgl_hotload allows the user to use custom allocators by redefining:
 *
 *     #define HGL_HOTLOAD_ALLOC       malloc
 *     #define HGL_HOTLOAD_REALLOCATOR realloc
 *     #define HGL_HOTLOAD_FREE        free
 *
 *
 * EXAMPLE:
 *
 * In this example we use hgl_hotload to hotload a simple print-function with the prototype 
 * 'void mylib_print(void);' from an the external library 'libmylib.so'. Error checking has 
 * been omitted for the sake of brevity:
 *     
 *     #define HGL_HOTLOAD_IMPLEMENTATION
 *     #include "hgl_hotload.h"
 *     
 *     typedef void (*mylib_print_t)(void);
 *     static mylib_print_t mylib_print = NULL; // our function handle
 *     
 *     int main(void)
 *     {
 *         hgl_hotload_init(HGL_HOTLOAD_LIBS("libmylib.so"), RTLD_NOW);
 *         hgl_hotload_add_symbol((void *) &mylib_print, "libmylib.so", "mylib_print"); 
 *         hgl_hotload_force_reload_all();
 *         for (int i = 0; i < 100; i++) {
 *             usleep(1000000);
 *             printf("Hello from main program! %d \n", i);
 *             mylib_print();
 *             hgl_hotload_reload_lib("libmylib.so"); 
 *         }
 *         hgl_hotload_final();
 *     }
 *
 * Note: the example above might crash since "libmylib.so" may be reopened while someone 
 * is writing to it (such as a compiler... ). A safer way to handle reloading would be to 
 * put 'hgl_hotload_reload_lib' behind some event, such as a keypress.
 *
 * AUTHOR: Henrik A. Glass
 *
 */

#ifndef HGL_HOTLOAD_H
#define HGL_HOTLOAD_H

/*--- Include files ---------------------------------------------------------------------*/

#include <stdio.h>
#include <stdint.h>
#include <assert.h>
#include <errno.h>
#include <dlfcn.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <time.h>

/*--- Public macros ---------------------------------------------------------------------*/

/* construct an in-place array for passing to hgl_hotload_init */
#define HGL_HOTLOAD_LIBS(...) ((const char *[]){ __VA_ARGS__ , NULL})

/* CONFIGURABLE: HGL_HOTLOAD_ALLOC, HGL_HOTLOAD_FREE */
#if !defined(HGL_HOTLOAD_ALLOC) && \
    !defined(HGL_HOTLOAD_REALLOCATOR) && \
    !defined(HGL_HOTLOAD_FREE)
#include <stdlib.h>
#define HGL_HOTLOAD_ALLOC        malloc
#define HGL_HOTLOAD_REALLOCATOR  realloc
#define HGL_HOTLOAD_FREE         free
#endif

/*--- Public type definitions -----------------------------------------------------------*/

typedef struct {
    char *name;
    void **ptr_to_handle;
} HglSymbol;

typedef struct {
    char *path;
    HglSymbol *symbols;
    int n_symbols;
    void *dl_handle;
    time_t time_of_last_modification;
} HglLibInfo;

typedef struct {
    HglLibInfo *libs;
    int n_libs;
    int flags;
} HglHotloadContext;

/**
 * Like strdup, but uses HGL_HOTLOAD_ALLOC instead of malloc. Intended for internal use.
 */
char *hgl_hotload_strdup_(const char *str);

/**
 * Unloads all hot-loaded libraries and performs cleanup. Uninitializes the global 
 * HglHotloadContext.
 */
void hgl_hotload_final(void);

/**
 * Initializes the global HglHotloadContext. 'lib_paths' takes an array of c strings with 
 * the paths of the hotloadable libraries. The last element of 'lib_paths' must be a NULL
 * pointer. The HGL_HOTLOAD_LIBS() function-like macro may be used to construct this list 
 * in-place. The 'flags' parameter is passed to all calls to dlopen made internally by 
 * hgl_hotload. See 'man 3 dlopen' for details on the 'flags' parameter.
 *
 * returns 0 on success, -1 otherwise.
 */
int hgl_hotload_init(const char *lib_paths[], int flags);

/**
 * Associates a library path and symbol name with a pointer to a symbol handle. 
 * 'ptr_to_symbol_handle' is a pointer to a pointer to a symbol. For instance, 
 * 'ptr_to_symbol_handle' could be a pointer to a function pointer. 'lib_path' and 
 * 'symbol_name' tell hgl_hotload where to look for a specific symbol.
 *
 * NB: 'hgl_hotload_add_symbol' does not load any symbols. It simply creates an association 
 * between symbol names and symbol handles that are used in subsequent calls to 
 * 'hgl_hotload_force_reload_all' and 'hgl_hotload_reload_lib'.
 *
 * returns 0 on success, -1 otherwise.
 */
int hgl_hotload_add_symbol(void **ptr_to_symbol_handle, 
                           const char *lib_path,
                           const char *symbol_name);

/**
 * Forces a reload of all libraries (and all symbols) managed by hgl_hotload, regardless of
 * if they've been modified since the last load.
 *
 * returns 0 on success, -1 otherwise.
 */
int hgl_hotload_force_reload_all(void);

/**
 * Tries to reload all symbols associated with the library 'lib_path'. If the library has not
 * been modified since the last load no reloading takes place.
 *
 * returns 0 on success, -1 otherwise.
 */
int hgl_hotload_reload_lib(const char *lib_path);

#endif /* HGL_HOTLOAD_H */

#ifdef HGL_HOTLOAD_IMPLEMENTATION

HglHotloadContext hgl_hl_global_context;

char *hgl_hotload_strdup_(const char *str)
{
    size_t alloc_size = strlen(str) + 1;
    char *cpy = HGL_HOTLOAD_ALLOC(alloc_size);
    memset(cpy, 0, alloc_size);
    strcpy(cpy, str);
    return cpy;
}

void hgl_hotload_final()
{
    if (hgl_hl_global_context.libs == NULL) {
        return;
    }

    // iterate hgl_hl_global_context.libs and closedown stuff.
    for (int i = 0; i < hgl_hl_global_context.n_libs; i++) {
        if (hgl_hl_global_context.libs[i].dl_handle != NULL) {
            HglLibInfo *libinfo = &hgl_hl_global_context.libs[i];
#ifdef HGL_HOTLOAD_DEBUG_PRINTS
            printf("[hgl_hotload] Unload: %s\n", libinfo->path);
#endif
            for (int j = 0; j < libinfo->n_symbols; j++) {
                *libinfo->symbols[j].ptr_to_handle = NULL;
                HGL_HOTLOAD_FREE(libinfo->symbols[j].name);
            }
            HGL_HOTLOAD_FREE(libinfo->symbols);

            dlclose(libinfo->dl_handle);
            HGL_HOTLOAD_FREE(libinfo->path);
        }
    }

    HGL_HOTLOAD_FREE(hgl_hl_global_context.libs);
}

int hgl_hotload_init(const char *lib_paths[], int flags)
{
    hgl_hl_global_context.libs   = NULL;
    hgl_hl_global_context.n_libs = 0;
    hgl_hl_global_context.flags  = flags;

    if (lib_paths == NULL) {
        fprintf(stderr, "Error: No library paths supplied to hgl_hotload_init. <%s, %d>\n", __FILE__, __LINE__);
        return -1;
    }

    for (int i = 0; lib_paths[i] != NULL; i++) hgl_hl_global_context.n_libs++;
    hgl_hl_global_context.libs = HGL_HOTLOAD_ALLOC(hgl_hl_global_context.n_libs * sizeof(HglLibInfo));
    if(hgl_hl_global_context.libs == NULL) {
        fprintf(stderr, "Error: HGL_HOTLOAD_ALLOC failed. Buy more RAM. <%s, %d>\n", __FILE__, __LINE__);
        return -1;
    }

    for (int i = 0; i < hgl_hl_global_context.n_libs; i++) {
        HglLibInfo *libinfo = &hgl_hl_global_context.libs[i];
        libinfo->n_symbols = 0;
        libinfo->path = hgl_hotload_strdup_(lib_paths[i]); 
        libinfo->dl_handle = dlopen(hgl_hl_global_context.libs[i].path, flags);
        libinfo->time_of_last_modification = 0;
        if(libinfo->dl_handle == NULL) {
            fprintf(stderr, "Error: dlopen failed on %s. dlerror = %s <%s, %d>\n", 
                    libinfo->path, dlerror(), __FILE__, __LINE__);
            hgl_hotload_final();
            return -1;
        }
        assert(hgl_hl_global_context.libs[i].dl_handle != NULL);
#ifdef HGL_HOTLOAD_DEBUG_PRINTS
        printf("[hgl_hotload] Load: %s\n", libinfo->path);
#endif
    } 

    return 0;
}

int hgl_hotload_add_symbol(void **ptr_to_symbol_handle, 
                           const char *lib_path,
                           const char *symbol_name)
{
    int i = 0;
    while ((i < hgl_hl_global_context.n_libs) && 
           (strcmp(hgl_hl_global_context.libs[i].path, lib_path) != 0)) i++;
    HglLibInfo *libinfo = &hgl_hl_global_context.libs[i];

    i = 0;
    while (i < libinfo->n_symbols && strcmp(libinfo->symbols[i].name, symbol_name) != 0) i++;
    if (i >= libinfo->n_symbols) {
        libinfo->n_symbols++;
        libinfo->symbols = HGL_HOTLOAD_REALLOCATOR(libinfo->symbols, libinfo->n_symbols * sizeof(HglSymbol));
        if(libinfo->symbols == NULL) {
            fprintf(stderr, "Error: HGL_HOTLOAD_ALLOC failed. Buy more RAM. <%s, %d>\n", __FILE__, __LINE__);
            return -1;
        }
    }

    HglSymbol *symbol     = &libinfo->symbols[i];
    symbol->name          = hgl_hotload_strdup_(symbol_name);
    symbol->ptr_to_handle = ptr_to_symbol_handle;

    return 0;
}

int hgl_hotload_force_reload_all()
{
    for (int i = 0; i < hgl_hl_global_context.n_libs; i++) {
        /* close and reopen the handle */
        HglLibInfo *libinfo = &hgl_hl_global_context.libs[i];
        if (libinfo->dl_handle != NULL) {
            dlclose(libinfo->dl_handle);
        }
        libinfo->dl_handle = dlopen(libinfo->path, hgl_hl_global_context.flags);
        if (libinfo->dl_handle == NULL) {
            fprintf(stderr, "Error: Failed to reopen dl. dlerror = %s <%s, %d>\n", 
                    dlerror(), __FILE__, __LINE__);
            return -1;
        }

        /* update timespec */
        struct stat statbuf;
        int err = stat(libinfo->path, &statbuf);
        if (err != 0) {
            fprintf(stderr, "Error: \"%s\" <%s, %d>\n", 
                    strerror(errno), __FILE__, __LINE__);
            return -1;
        }
        libinfo->time_of_last_modification = statbuf.st_mtime;
    
        /* update symbols */
        for (int j = 0; j < libinfo->n_symbols; j++) {
            *libinfo->symbols[j].ptr_to_handle = dlsym(libinfo->dl_handle, libinfo->symbols[j].name);
            if (*libinfo->symbols[j].ptr_to_handle == NULL) {
                fprintf(stderr, "Error: get symbol \"%s\". dlerror = %s <%s, %d>\n", 
                        libinfo->symbols[i].name, dlerror(), __FILE__, __LINE__);
                return -1;
            }
        }
#ifdef HGL_HOTLOAD_DEBUG_PRINTS
        printf("[hgl_hotload] Reload: %s\n", libinfo->path);
#endif
    }

    return 0;
}

int hgl_hotload_reload_lib(const char *lib_path)
{
    /* find the handle for 'lib_path'*/
    int i = 0;
    while ((i < hgl_hl_global_context.n_libs) && 
           (strcmp(hgl_hl_global_context.libs[i].path, lib_path) != 0)) i++;
    HglLibInfo *libinfo = &hgl_hl_global_context.libs[i];

    /* compare modification date. Load only if lib has been modified */
    struct stat statbuf; 
    int err = stat(libinfo->path, &statbuf);
    if (err != 0) {
        fprintf(stderr, "Error: \"%s\" <%s, %d>\n", 
                strerror(errno), __FILE__, __LINE__);
        return -1;
    }
    if (libinfo->time_of_last_modification == statbuf.st_mtime) {
        return 0; // no modification since last load.
    }
    libinfo->time_of_last_modification = statbuf.st_mtime;
    

    /* close and reopen the handle */
    if (libinfo->dl_handle != NULL) {
        dlclose(libinfo->dl_handle);
    }
    libinfo->dl_handle = dlopen(libinfo->path, hgl_hl_global_context.flags);
    if (libinfo->dl_handle == NULL) {
        fprintf(stderr, "Error: Failed to reopen dl. dlerror = %s <%s, %d>\n", 
                dlerror(), __FILE__, __LINE__);
        return -1;
    }

    /* update symbols */
    for (i = 0; i < libinfo->n_symbols; i++) {
        *libinfo->symbols[i].ptr_to_handle = dlsym(libinfo->dl_handle, libinfo->symbols[i].name);
        if (*libinfo->symbols[i].ptr_to_handle == NULL) {
            fprintf(stderr, "Error: get symbol \"%s\". dlerror = %s <%s, %d>\n", 
                    libinfo->symbols[i].name, dlerror(), __FILE__, __LINE__);
            return -1;
        }
    }

#ifdef HGL_HOTLOAD_DEBUG_PRINTS
        printf("[hgl_hotload] Reload: %s\n", libinfo->path);
#endif

    return 0;
}

#endif /* HGL_HOTLOAD_IMPLEMENTATION */

