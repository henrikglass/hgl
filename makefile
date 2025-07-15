
.PHONY: hotload_mylib test prep

SHELL := /bin/bash

MAKEFLAGS := --jobs=$(shell nproc)

EXAMPLES_DIR := examples
TEST_DIR := test
EXAMPLES_BUILD_DIR := build/examples
TEST_BUILD_DIR := build/test

all: examples test

examples: vec         					\
          rbuf                          \
          chan                          \
          rhashtbl                      \
          htable                        \
          hset                          \
          stack_alloc                   \
          arena_alloc                   \
          pool_alloc                    \
          fs_alloc                      \
          misc                          \
          hotload                       \
          serialize                     \
          memdbg                        \
          flags                         \
          string                        \
          hamming                       \
          cmd                           \
          io                            \
          process                       \
          profile                       \
          hglm                          \
          fft                           \
          ini                           \
          rle                           \
          worker_pool                   \
          barrier                       \
          rbtree                        \
          sockets                       \
          queue                         \
          base64                        \
          rita_castle                   \
          rita_hello_triangle           \
          rita_hello_cube               \
          rita_primitives               \
          rita_3d                       \
          rita_filters                  \
          rita_game_of_life             \
          rita_blit                     \
          rita_custom_fragment_shader   \
          rita_skybox                   \
          rita_vertex_displacement      \
          rita_vertex_displacement_maps \
          rita_pebbles                  \
          rita_mandelbulb               \
   	  tqueue      

misc: prep
	gcc -I. -std=c17 -Wall -Wextra -Werror -O0 -ggdb3 -D_POSIX_C_SOURCE=199309L $(EXAMPLES_DIR)/misc.c -o $(EXAMPLES_BUILD_DIR)/misc

vec: prep
	gcc -I. -std=c17 -Wall -Wextra -Wpedantic -Werror -O0 -ggdb3 $(EXAMPLES_DIR)/vec.c -o $(EXAMPLES_BUILD_DIR)/vec

rbuf: prep
	gcc -I. -std=c17 -Wall -Wextra -Wpedantic -Werror -O0 -ggdb3 $(EXAMPLES_DIR)/rbuf.c -o $(EXAMPLES_BUILD_DIR)/rbuf

chan: prep
	gcc -I. -std=c17 -Wall -Wextra -Wpedantic -Werror -O0 -ggdb3 $(EXAMPLES_DIR)/chan.c -o $(EXAMPLES_BUILD_DIR)/chan

rhashtbl: prep
	gcc -I. -std=c17 -Wall -Wextra -Wpedantic -Werror -O0 -ggdb3 $(EXAMPLES_DIR)/rhashtbl.c -o $(EXAMPLES_BUILD_DIR)/rhashtbl

htable: prep
	gcc -I. -std=c17 -Wall -Wextra -Wpedantic -Werror -O0 -ggdb3 $(EXAMPLES_DIR)/htable.c -o $(EXAMPLES_BUILD_DIR)/htable

hset: prep
	gcc -I. -std=c17 -Wall -Wextra -Wpedantic -Werror -O0 -ggdb3 $(EXAMPLES_DIR)/hset.c -o $(EXAMPLES_BUILD_DIR)/hset

stack_alloc: prep
	gcc -I. -std=c17 -Wall -Wextra -Wpedantic -Werror -O0 -D_DEFAULT_SOURCE -ggdb3 $(EXAMPLES_DIR)/stack_alloc.c -o $(EXAMPLES_BUILD_DIR)/stack_alloc

arena_alloc: prep
	gcc -I. -std=c17 -Wall -Wextra -Wpedantic -Werror -O0 -D_DEFAULT_SOURCE -ggdb3 $(EXAMPLES_DIR)/arena_alloc.c -o $(EXAMPLES_BUILD_DIR)/arena_alloc

pool_alloc: prep
	gcc -I. -std=c17 -Wall -Wextra -Wpedantic -Werror -O0 -ggdb3 $(EXAMPLES_DIR)/pool_alloc.c -o $(EXAMPLES_BUILD_DIR)/pool_alloc

fs_alloc: prep
	gcc -I. -std=c17 -Wall -Wextra -Wpedantic -Werror -O0 -ggdb3 $(EXAMPLES_DIR)/fs_alloc.c -o $(EXAMPLES_BUILD_DIR)/fs_alloc

hotload: hotload_mylib prep
	gcc -I. -D_POSIX_C_SOURCE=200809L -std=c17 -Wall -Wextra -Wpedantic -Werror -O0 -ggdb3 -Wl,-rpath="./" $(EXAMPLES_DIR)/hotload.c -o $(EXAMPLES_BUILD_DIR)/hotload -ldl

hotload_mylib: prep
	gcc -I. -fPIE -shared -Wall -Wextra -Wpedantic -Wstrict-prototypes -Wmissing-declarations -Wmissing-prototypes -ggdb3 $(EXAMPLES_DIR)/mylib.c -o $(EXAMPLES_BUILD_DIR)/libmylib.so

serialize: prep
	gcc -I. -std=c17 -Wall -Wextra -Wpedantic -Werror -O0 -ggdb3 $(EXAMPLES_DIR)/serialize.c -o $(EXAMPLES_BUILD_DIR)/serialize
	gcc -I. -std=c17 -Wall -Wextra -Wpedantic -Werror -O0 -ggdb3 $(EXAMPLES_DIR)/serialize_elfinfo.c -o $(EXAMPLES_BUILD_DIR)/serialize_elfinfo

memdbg: prep
	gcc -I. -std=c17 -Wall -Wextra -Wpedantic -Wno-unused-variable -Werror -O0 -ggdb3 $(EXAMPLES_DIR)/memdbg.c -o $(EXAMPLES_BUILD_DIR)/memdbg

flags: prep
	gcc -I. -std=c17 -Wall -Wextra -Wpedantic -Wno-unused-variable -Werror -O0 -ggdb3 $(EXAMPLES_DIR)/flags.c -o $(EXAMPLES_BUILD_DIR)/flags -lm

string: prep
	gcc -I. -std=c17 -Wall -Wextra -Wpedantic -Wno-unused-variable -Werror -O0 -ggdb3 $(EXAMPLES_DIR)/string.c -o $(EXAMPLES_BUILD_DIR)/string

fft: prep
	gcc -I. -std=c17 -Wall -Wextra -Wpedantic -Wno-unused-variable -Werror -O3 -march=native -ffast-math $(EXAMPLES_DIR)/fft.c -o $(EXAMPLES_BUILD_DIR)/fft -lm

hamming: prep
	gcc -I. -std=c17 -Wall -Wextra -Wpedantic -Wno-unused-variable -Werror -O0 -ggdb3 $(EXAMPLES_DIR)/hamming.c -o $(EXAMPLES_BUILD_DIR)/hamming
	gcc -I. -std=c17 -Wall -Wextra -Wpedantic -Wno-unused-variable -Werror -O0 -D_DEFAULT_SOURCE -ggdb3 $(EXAMPLES_DIR)/hamming_encode.c -o $(EXAMPLES_BUILD_DIR)/hamming_encode
	gcc -I. -std=c17 -Wall -Wextra -Wpedantic -Wno-unused-variable -Werror -O0 -D_DEFAULT_SOURCE -ggdb3 $(EXAMPLES_DIR)/hamming_decode.c -o $(EXAMPLES_BUILD_DIR)/hamming_decode

io: prep
	gcc -I. -std=c17 -Wall -Wextra -Wno-unused-variable -Werror -O0 -D_DEFAULT_SOURCE -ggdb3 $(EXAMPLES_DIR)/io.c -o $(EXAMPLES_BUILD_DIR)/io

process: prep
	gcc -I. -std=c17 -Wall -Wextra -Wno-unused-variable -Werror -O0 -D_POSIX_C_SOURCE -ggdb3 $(EXAMPLES_DIR)/process.c -o $(EXAMPLES_BUILD_DIR)/process

cmd: prep
	gcc -I. -std=c17 -Wall -Wextra -Wpedantic -Wno-unused-variable -Werror -O0 -D_POSIX_C_SOURCE -ggdb3 $(EXAMPLES_DIR)/cmd.c -o $(EXAMPLES_BUILD_DIR)/cmd

profile: prep
	gcc -I. -std=c17 -Wall -Wextra -Wpedantic -Wno-unused-variable -Werror -O3 -D_POSIX_C_SOURCE -ggdb3 $(EXAMPLES_DIR)/profile.c -o $(EXAMPLES_BUILD_DIR)/profile

hglm: prep
	gcc -I. -std=c17 -Wall -Wextra -Wpedantic -Wno-unused-variable -Werror -O3 -ggdb3 $(EXAMPLES_DIR)/hglm.c -o $(EXAMPLES_BUILD_DIR)/hglm -lm
	gcc -I. -std=c17 -Wall -Wextra -Wpedantic -Wno-unused-variable -Werror -O3 -DHGLM_USE_SIMD -mavx -ggdb3 $(EXAMPLES_DIR)/hglm.c -o $(EXAMPLES_BUILD_DIR)/hglm_simd -lm

queue: prep
	gcc -I. -std=c17 -Wall -Wextra -Wno-unused-variable -Werror -O0 -D_POSIX_C_SOURCE -ggdb3 $(EXAMPLES_DIR)/queue.c -o $(EXAMPLES_BUILD_DIR)/queue

tqueue: prep
	gcc -I. -std=c17 -Wall -Wextra -Wno-unused-variable -Werror -O0 -D_POSIX_C_SOURCE=199309L -ggdb3 $(EXAMPLES_DIR)/tq.c -o $(EXAMPLES_BUILD_DIR)/tq

ini: prep
	gcc -I. -std=c17 -Wall -Wextra -Wno-unused-variable -Werror -O0 -ggdb3 $(EXAMPLES_DIR)/ini.c -o $(EXAMPLES_BUILD_DIR)/ini

rle: prep
	gcc -I. -std=c17 -Wall -Wextra -Wno-unused-variable -Werror -O0 -ggdb3 $(EXAMPLES_DIR)/rle.c -o $(EXAMPLES_BUILD_DIR)/rle
	gcc -I. -std=c17 -Wall -Wextra -Wno-unused-variable -Werror -O0 -ggdb3 $(EXAMPLES_DIR)/rle_hglzip.c -o $(EXAMPLES_BUILD_DIR)/hglzip

worker_pool: prep
	gcc -I. -std=c17 -Wall -Wextra -Wno-unused-variable -Werror -O0 -ggdb3 -D_POSIX_C_SOURCE=199309L -fsanitize=thread $(EXAMPLES_DIR)/worker_pool.c -o $(EXAMPLES_BUILD_DIR)/worker_pool -lpthread

barrier: prep
	gcc -I. -std=c17 -Wall -Wextra -Wno-unused-variable -Werror -O0 -ggdb3 -D_POSIX_C_SOURCE=199309L -fsanitize=thread $(EXAMPLES_DIR)/barrier.c -o $(EXAMPLES_BUILD_DIR)/barrier -lpthread

rbtree: prep
	gcc -I. -std=c17 -Wall -Wextra -Wno-unused-variable -Werror -O0 -ggdb3 $(EXAMPLES_DIR)/rbtree.c -o $(EXAMPLES_BUILD_DIR)/rbtree

base64: prep
	gcc -I. -std=c17 -Wall -Wextra -Wno-unused-variable -Werror -O0 -ggdb3 $(EXAMPLES_DIR)/base64.c -o $(EXAMPLES_BUILD_DIR)/base64

sockets: prep
	gcc -I. -std=c17 -Wall -Wextra -Wno-unused-variable -Werror -O0 -ggdb3 -D_POSIX_C_SOURCE=200809L $(EXAMPLES_DIR)/sockets.c -o $(EXAMPLES_BUILD_DIR)/sockets

rita_castle: prep
	gcc -I. -Iinclude -std=c17 -Wall -Wextra -Werror -Wshadow -Wdouble-promotion -O0 -ggdb3 $(EXAMPLES_DIR)/rita_castle.c -o $(EXAMPLES_BUILD_DIR)/rita_castle -Llib -lraylib -lm -ldl -lpthread
	gcc -I. -Iinclude -std=c17 -Wall -Wextra -Werror -Wshadow -Wdouble-promotion -Ofast -march=native -ffast-math $(EXAMPLES_DIR)/rita_castle.c -o $(EXAMPLES_BUILD_DIR)/rita_castle_optimized -Llib -lraylib -lm -ldl -lpthread

rita_hello_triangle: prep
	gcc -I. -Iinclude -std=c17 -Wall -Wextra -Werror -Wshadow -Wdouble-promotion -O0 -ggdb3 $(EXAMPLES_DIR)/rita_hello_triangle.c -o $(EXAMPLES_BUILD_DIR)/rita_hello_triangle -Llib -lraylib -lm -ldl -lpthread
	gcc -I. -Iinclude -std=c17 -Wall -Wextra -Werror -Wshadow -Wdouble-promotion -Ofast -march=native -ffast-math $(EXAMPLES_DIR)/rita_hello_triangle.c -o $(EXAMPLES_BUILD_DIR)/rita_hello_triangle_optimized -Llib -lraylib -lm -ldl -lpthread

rita_hello_cube: prep
	gcc -I. -Iinclude -std=c17 -Wall -Wextra -Werror -Wshadow -Wdouble-promotion -O0 -ggdb3 $(EXAMPLES_DIR)/rita_hello_cube.c -o $(EXAMPLES_BUILD_DIR)/rita_hello_cube -Llib -lraylib -lm -ldl -lpthread
	gcc -I. -Iinclude -std=c17 -Wall -Wextra -Werror -Wshadow -Wdouble-promotion -Ofast -march=native -ffast-math $(EXAMPLES_DIR)/rita_hello_cube.c -o $(EXAMPLES_BUILD_DIR)/rita_hello_cube_optimized -Llib -lraylib -lm -ldl -lpthread

rita_primitives: prep
	gcc -I. -Iinclude -std=c17 -Wall -Wextra -Werror -Wshadow -Wdouble-promotion -O0 -ggdb3 $(EXAMPLES_DIR)/rita_primitives.c -o $(EXAMPLES_BUILD_DIR)/rita_primitives -Llib -lraylib -lm -ldl -lpthread
	gcc -I. -Iinclude -std=c17 -Wall -Wextra -Werror -Wshadow -Wdouble-promotion -Ofast -march=native -ffast-math $(EXAMPLES_DIR)/rita_primitives.c -o $(EXAMPLES_BUILD_DIR)/rita_primitives_optimized -Llib -lraylib -lm -ldl -lpthread

rita_3d: prep
	gcc -I. -Iinclude -std=c17 -Wall -Wextra -Werror -Wshadow -Wdouble-promotion -O0 -ggdb3 $(EXAMPLES_DIR)/rita_3d.c -o $(EXAMPLES_BUILD_DIR)/rita_3d -Llib -lraylib -lm -ldl -lpthread
	gcc -I. -Iinclude -std=c17 -Wall -Wextra -Werror -Wshadow -Wdouble-promotion -Ofast -march=native -ffast-math $(EXAMPLES_DIR)/rita_3d.c -o $(EXAMPLES_BUILD_DIR)/rita_3d_optimized -Llib -lraylib -lm -ldl -lpthread

rita_filters: prep
	gcc -I. -Iinclude -std=c17 -Wall -Wextra -Werror -Wshadow -Wdouble-promotion -O0 -ggdb3 $(EXAMPLES_DIR)/rita_filters.c -o $(EXAMPLES_BUILD_DIR)/rita_filters -Llib -lraylib -lm -ldl -lpthread
	gcc -I. -Iinclude -std=c17 -Wall -Wextra -Werror -Wshadow -Wdouble-promotion -Ofast -march=native -ffast-math $(EXAMPLES_DIR)/rita_filters.c -o $(EXAMPLES_BUILD_DIR)/rita_filters_optimized -Llib -lraylib -lm -ldl -lpthread

rita_game_of_life: prep
	gcc -I. -Iinclude -std=c17 -Wall -Wextra -Werror -Wshadow -Wdouble-promotion -O0 -ggdb3 $(EXAMPLES_DIR)/rita_game_of_life.c -o $(EXAMPLES_BUILD_DIR)/rita_game_of_life -Llib -lraylib -lm -ldl -lpthread
	gcc -I. -Iinclude -std=c17 -Wall -Wextra -Werror -Wshadow -Wdouble-promotion -Ofast -march=native -ffast-math $(EXAMPLES_DIR)/rita_game_of_life.c -o $(EXAMPLES_BUILD_DIR)/rita_game_of_life_optimized -Llib -lraylib -lm -ldl -lpthread

rita_blit: prep
	gcc -I. -Iinclude -std=c17 -Wall -Wextra -Werror -Wshadow -Wdouble-promotion -O0 -ggdb3 $(EXAMPLES_DIR)/rita_blit.c -o $(EXAMPLES_BUILD_DIR)/rita_blit -Llib -lraylib -lm -ldl -lpthread
	gcc -I. -Iinclude -std=c17 -Wall -Wextra -Werror -Wshadow -Wdouble-promotion -Ofast -march=native -ffast-math $(EXAMPLES_DIR)/rita_blit.c -o $(EXAMPLES_BUILD_DIR)/rita_blit_optimized -Llib -lraylib -lm -ldl -lpthread

rita_custom_fragment_shader: prep
	gcc -I. -Iinclude -std=c17 -Wall -Wextra -Werror -Wshadow -Wdouble-promotion -O0 -ggdb3 $(EXAMPLES_DIR)/rita_custom_fragment_shader.c -o $(EXAMPLES_BUILD_DIR)/rita_custom_fragment_shader -Llib -lraylib -lm -ldl -lpthread
	gcc -I. -Iinclude -std=c17 -Wall -Wextra -Werror -Wshadow -Wdouble-promotion -Ofast -march=native -ffast-math $(EXAMPLES_DIR)/rita_custom_fragment_shader.c -o $(EXAMPLES_BUILD_DIR)/rita_custom_fragment_shader_optimized -Llib -lraylib -lm -ldl -lpthread

rita_skybox: prep
	gcc -I. -Iinclude -std=c17 -Wall -Wextra -Werror -Wshadow -Wdouble-promotion -O0 -ggdb3 $(EXAMPLES_DIR)/rita_skybox.c -o $(EXAMPLES_BUILD_DIR)/rita_skybox -Llib -lraylib -lm -ldl -lpthread
	gcc -I. -Iinclude -std=c17 -Wall -Wextra -Werror -Wshadow -Wdouble-promotion -Ofast -march=native -ffast-math $(EXAMPLES_DIR)/rita_skybox.c -o $(EXAMPLES_BUILD_DIR)/rita_skybox_optimized -Llib -lraylib -lm -ldl -lpthread

rita_vertex_displacement: prep
	gcc -I. -Iinclude -std=c17 -Wall -Wextra -Werror -Wshadow -Wdouble-promotion -O0 -ggdb3 $(EXAMPLES_DIR)/rita_vertex_displacement.c -o $(EXAMPLES_BUILD_DIR)/rita_vertex_displacement -Llib -lraylib -lm -ldl -lpthread
	gcc -I. -Iinclude -std=c17 -Wall -Wextra -Werror -Wshadow -Wdouble-promotion -Ofast -march=native -ffast-math $(EXAMPLES_DIR)/rita_vertex_displacement.c -o $(EXAMPLES_BUILD_DIR)/rita_vertex_displacement_optimized -Llib -lraylib -lm -ldl -lpthread

rita_vertex_displacement_maps: prep
	gcc -I. -Iinclude -std=c17 -Wall -Wextra -Werror -Wshadow -Wdouble-promotion -O0 -ggdb3 $(EXAMPLES_DIR)/rita_vertex_displacement_maps.c -o $(EXAMPLES_BUILD_DIR)/rita_vertex_displacement_maps -Llib -lraylib -lm -ldl -lpthread
	gcc -I. -Iinclude -std=c17 -Wall -Wextra -Werror -Wshadow -Wdouble-promotion -Ofast -march=native -ffast-math $(EXAMPLES_DIR)/rita_vertex_displacement_maps.c -o $(EXAMPLES_BUILD_DIR)/rita_vertex_displacement_maps_optimized -Llib -lraylib -lm -ldl -lpthread

rita_pebbles: prep
	gcc -I. -Iinclude -std=c17 -Wall -Wextra -Werror -Wshadow -Wdouble-promotion -O0 -ggdb3 $(EXAMPLES_DIR)/rita_pebbles.c -o $(EXAMPLES_BUILD_DIR)/rita_pebbles -Llib -lraylib -lm -ldl -lpthread
	gcc -I. -Iinclude -std=c17 -Wall -Wextra -Werror -Wshadow -Wdouble-promotion -Ofast -march=native -ffast-math $(EXAMPLES_DIR)/rita_pebbles.c -o $(EXAMPLES_BUILD_DIR)/rita_pebbles_optimized -Llib -lraylib -lm -ldl -lpthread

rita_mandelbulb: prep
	gcc -I. -Iinclude -std=c17 -Wall -Wextra -Werror -Wshadow -Wdouble-promotion -O0 -ggdb3 $(EXAMPLES_DIR)/rita_mandelbulb.c include/ffmpeg_linux.c -o $(EXAMPLES_BUILD_DIR)/rita_mandelbulb -Llib -lraylib -lm -ldl -lpthread
	gcc -I. -Iinclude -std=c17 -Wall -Wextra -Werror -Wshadow -Wdouble-promotion -Ofast -march=native -ffast-math $(EXAMPLES_DIR)/rita_mandelbulb.c include/ffmpeg_linux.c -o $(EXAMPLES_BUILD_DIR)/rita_mandelbulb_optimized -Llib -lraylib -lm -ldl -lpthread


## Unit tests
test: prep
	gcc -I. -std=c17 -Wall -Wextra -Wno-unused-variable -Werror -O0 -D_DEFAULT_SOURCE -ggdb3 $(TEST_DIR)/test_arena_alloc.c -o $(TEST_BUILD_DIR)/test_arena_alloc
	gcc -I. -std=c17 -Wall -Wextra -Wno-unused-variable -Werror -O0 -D_DEFAULT_SOURCE -DHGL_ARENA_ALLOW_EXPENSIVE_REALLOC -ggdb3 $(TEST_DIR)/test_arena_alloc.c -o $(TEST_BUILD_DIR)/test_arena_alloc2
	gcc -I. -std=c17 -Wall -Wextra -Wno-unused-variable -Werror -O0 -D_DEFAULT_SOURCE -ggdb3 $(TEST_DIR)/test_stack_alloc.c -o $(TEST_BUILD_DIR)/test_stack_alloc
	gcc -I. -std=c17 -Wall -Wextra -Wno-unused-variable -Werror -O0 -ggdb3 $(TEST_DIR)/test_pool_alloc.c -o $(TEST_BUILD_DIR)/test_pool_alloc
	gcc -I. -std=c17 -Wall -Wextra -Wno-unused-variable -Werror -O0 -ggdb3 $(TEST_DIR)/test_fs_alloc.c -o $(TEST_BUILD_DIR)/test_fs_alloc
	gcc -I. -std=c17 -Wall -Wextra -Wno-unused-variable -Werror -O0 -ggdb3 $(TEST_DIR)/test_memdbg.c -o $(TEST_BUILD_DIR)/test_memdbg
	gcc -I. -std=c17 -Wall -Wextra -Wno-unused-variable -Werror -O0 -ggdb3 $(TEST_DIR)/test_chan.c -o $(TEST_BUILD_DIR)/test_chan
	gcc -I. -std=c17 -Wall -Wextra -Wno-unused-variable -Werror -O0 -ggdb3 $(TEST_DIR)/test_buffered_chan.c -o $(TEST_BUILD_DIR)/test_buffered_chan
	gcc -I. -std=c17 -Wall -Wextra -Wno-unused-variable -Werror -O0 -ggdb3 $(TEST_DIR)/test_string.c -o $(TEST_BUILD_DIR)/test_string
	gcc -I. -std=c17 -Wall -Wextra -Wno-unused-variable -Werror -O0 -ggdb3 $(TEST_DIR)/test_da.c -o $(TEST_BUILD_DIR)/test_da
	gcc -I. -std=c17 -Wall -Wextra -Wno-unused-variable -Werror -O0 -ggdb3 $(TEST_DIR)/test_q.c -o $(TEST_BUILD_DIR)/test_q
	gcc -I. -std=c17 -Wall -Wextra -Wno-unused-variable -Werror -O0 -ggdb3 $(TEST_DIR)/test_htable.c -o $(TEST_BUILD_DIR)/test_htable
	gcc -I. -std=c17 -Wall -Wextra -Wno-unused-variable -Werror -O0 -ggdb3 $(TEST_DIR)/test_hset.c -o $(TEST_BUILD_DIR)/test_hset
	gcc -I. -std=c17 -Wall -Wextra -Wno-unused-variable -Werror -O0 -ggdb3 $(TEST_DIR)/test_ini.c -o $(TEST_BUILD_DIR)/test_ini
	gcc -I. -std=c17 -Wall -Wextra -Wno-unused-variable -Werror -O0 -ggdb3 $(TEST_DIR)/test_worker_pool.c -o $(TEST_BUILD_DIR)/test_worker_pool -lpthread
	gcc -I. -std=c17 -Wall -Wextra -Wno-unused-variable -Werror -O0 -ggdb3 $(TEST_DIR)/test_barrier.c -o $(TEST_BUILD_DIR)/test_barrier -lpthread
	gcc -I. -std=c17 -Wall -Wextra -Wno-unused-variable -Werror -O0 -ggdb3 $(TEST_DIR)/test_fft.c -o $(TEST_BUILD_DIR)/test_fft -lm
	gcc -I. -std=c17 -Wall -Wextra -Wno-unused-variable -Werror -O0 -ggdb3 $(TEST_DIR)/test_fft_simd.c -o $(TEST_BUILD_DIR)/test_fft_simd -lm
	gcc -I. -std=c17 -Wall -Wextra -Wno-unused-variable -Werror -O0 -ggdb3 $(TEST_DIR)/test_serialize.c -o $(TEST_BUILD_DIR)/test_serialize
	gcc -I. -std=c17 -Wall -Wextra -Wno-unused-variable -Werror -O0 -ggdb3 $(TEST_DIR)/test_rbtree.c -o $(TEST_BUILD_DIR)/test_rbtree
	gcc -I. -std=c17 -Wall -Wextra -Wno-unused-variable -Werror -O0 -ggdb3 $(TEST_DIR)/test_hamming.c -o $(TEST_BUILD_DIR)/test_hamming
	gcc -I. -std=c17 -Wall -Wextra -Wno-unused-variable -Werror -O0 -ggdb3 $(TEST_DIR)/test_flags.c -o $(TEST_BUILD_DIR)/test_flags
	gcc -I. -std=c17 -Wall -Wextra -Wno-unused-variable -Werror -O0 -ggdb3 $(TEST_DIR)/test_hglm.c -o $(TEST_BUILD_DIR)/test_hglm -lm
	gcc -I. -std=c17 -Wall -Wextra -Wno-unused-variable -Werror -O0 -ggdb3 $(TEST_DIR)/test_cmd.c -o $(TEST_BUILD_DIR)/test_cmd -lm
	gcc -I. -std=c17 -Wall -Wextra -Wno-unused-variable -Werror -O0 -ggdb3 $(TEST_DIR)/test_sockets.c -o $(TEST_BUILD_DIR)/test_sockets -lpthread
	gcc -I. -std=c17 -Wall -Wextra -Wno-unused-variable -Werror -O0 -ggdb3 $(TEST_DIR)/test_rle.c -o $(TEST_BUILD_DIR)/test_rle
	gcc -I. -std=c17 -Wall -Wextra -Wno-unused-variable -Werror -O0 -ggdb3 $(TEST_DIR)/test_tq.c -o $(TEST_BUILD_DIR)/test_tq
	gcc -I. -std=c17 -Wall -Wextra -Wno-unused-variable -Werror -O0 -ggdb3 $(TEST_DIR)/test_base64.c -o $(TEST_BUILD_DIR)/test_base64
	gcc -I. -std=c17 -Wall -Wextra -Wno-unused-variable -Werror -O0 -ggdb3 $(TEST_DIR)/test_waitgroup.c -o $(TEST_BUILD_DIR)/test_waitgroup
	gcc -I. -std=c17 -Wall -Wextra -Wno-unused-variable -Werror -O0 -ggdb3 $(TEST_DIR)/test_float.c -o $(TEST_BUILD_DIR)/test_float -lm
	gcc -I. -std=c17 -Wall -Wextra -Wno-unused-variable -Werror -O0 -ggdb3 $(TEST_DIR)/test_mem.c -o $(TEST_BUILD_DIR)/test_mem
	-rm run_tests.sh
	echo "#!/bin/bash" >> run_tests.sh
	find $(shell pwd)/build/test/ -type f -executable | sed "s/$$/ \&\&/">> run_tests.sh
	echo "exit 0;" >> run_tests.sh
	chmod +x run_tests.sh
	./run_tests.sh

prep:
	-mkdir build/
	-mkdir build/test
	-mkdir build/examples

clean:
	-rm $(EXAMPLES_BUILD_DIR)/*
	-rm $(TEST_BUILD_DIR)/*
	-rm run_tests.sh
	-rm -r build/*
	-rmdir build
