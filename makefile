.PHONY: test_hotload_mylib

MAKEFLAGS := --jobs=$(shell nproc)

SRCDIR := test

all: test_vec         \
     test_rbuf        \
     test_chan        \
     test_rhashtbl    \
     test_htable      \
     test_hset        \
     test_stack_alloc \
     test_arena_alloc \
     test_pool_alloc  \
     test_fs_alloc    \
     test_hgl         \
     test_hotload     \
     test_serialize   \
     test_memdbg      \
     test_flags       \
     test_string      \
     test_hamming     \
     test_cmd         \
     test_io          \
     test_process     \
     test_profile     \
     test_math        \
     test_fft         \
     test_ini         \
     test_worker_pool \
     test_rbtree 	  \
     test_queue

test_hgl:
	gcc -I. -std=c17 -Wall -Wextra -Werror -O0 -ggdb3 -D_POSIX_C_SOURCE=199309L $(SRCDIR)/test_hgl.c -o test_hgl

test_vec:
	gcc -I. -std=c17 -Wall -Wextra -Wpedantic -Werror -O0 -ggdb3 $(SRCDIR)/test_vec.c -o test_vec

test_rbuf:
	gcc -I. -std=c17 -Wall -Wextra -Wpedantic -Werror -O0 -ggdb3 $(SRCDIR)/test_rbuf.c -o test_rbuf

test_chan:
	gcc -I. -std=c17 -Wall -Wextra -Wpedantic -Werror -O0 -ggdb3 $(SRCDIR)/test_chan.c -o test_chan

test_rhashtbl:
	gcc -I. -std=c17 -Wall -Wextra -Wpedantic -Werror -O0 -ggdb3 $(SRCDIR)/test_rhashtbl.c -o test_rhashtbl

test_htable:
	gcc -I. -std=c17 -Wall -Wextra -Wpedantic -Werror -O0 -ggdb3 $(SRCDIR)/test_htable.c -o test_htable

test_hset:
	gcc -I. -std=c17 -Wall -Wextra -Wpedantic -Werror -O0 -ggdb3 $(SRCDIR)/test_hset.c -o test_hset

test_stack_alloc:
	gcc -I. -std=c17 -Wall -Wextra -Wpedantic -Werror -O0 -ggdb3 $(SRCDIR)/test_stack_alloc.c -o test_stack_alloc

test_arena_alloc:
	gcc -I. -std=c17 -Wall -Wextra -Wpedantic -Werror -O0 -ggdb3 $(SRCDIR)/test_arena_alloc.c -o test_arena_alloc

test_pool_alloc:
	gcc -I. -std=c17 -Wall -Wextra -Wpedantic -Werror -O0 -ggdb3 $(SRCDIR)/test_pool_alloc.c -o test_pool_alloc

test_fs_alloc:
	gcc -I. -std=c17 -Wall -Wextra -Wpedantic -Werror -O0 -ggdb3 $(SRCDIR)/test_fs_alloc.c -o test_fs_alloc

test_hotload: test_hotload_mylib
	gcc -I. -D_POSIX_C_SOURCE=200809L -std=c17 -Wall -Wextra -Wpedantic -Werror -O0 -ggdb3 -Wl,-rpath="./" $(SRCDIR)/test_hotload.c -o test_hotload -ldl

test_hotload_mylib:
	gcc -I. -fPIE -shared -Wall -Wextra -Wpedantic -Wstrict-prototypes -Wmissing-declarations -Wmissing-prototypes -ggdb3 $(SRCDIR)/mylib.c -o libmylib.so

test_serialize:
	gcc -I. -std=c17 -Wall -Wextra -Wpedantic -Werror -O0 -ggdb3 $(SRCDIR)/test_serialize.c -o test_serialize
	gcc -I. -std=c17 -Wall -Wextra -Wpedantic -Werror -O0 -ggdb3 $(SRCDIR)/test_serialize_elfinfo.c -o test_serialize_elfinfo

test_memdbg:
	gcc -I. -std=c17 -Wall -Wextra -Wpedantic -Wno-unused-variable -Werror -O0 -ggdb3 $(SRCDIR)/test_memdbg.c -o test_memdbg

test_flags:
	gcc -I. -std=c17 -Wall -Wextra -Wpedantic -Wno-unused-variable -Werror -O0 -ggdb3 $(SRCDIR)/test_flags.c -o test_flags -lm

test_string:
	gcc -I. -std=c17 -Wall -Wextra -Wpedantic -Wno-unused-variable -Werror -O0 -ggdb3 $(SRCDIR)/test_string.c -o test_string

test_fft:
	gcc -I. -std=c17 -Wall -Wextra -Wpedantic -Wno-unused-variable -Werror -O3 -march=native -ffast-math $(SRCDIR)/test_fft.c -o test_fft -lm

test_hamming:
	gcc -I. -std=c17 -Wall -Wextra -Wpedantic -Wno-unused-variable -Werror -O0 -ggdb3 $(SRCDIR)/test_hamming.c -o test_hamming
	gcc -I. -std=c17 -Wall -Wextra -Wpedantic -Wno-unused-variable -Werror -O0 -D_DEFAULT_SOURCE -ggdb3 $(SRCDIR)/test_hamming_encode.c -o test_hamming_encode
	gcc -I. -std=c17 -Wall -Wextra -Wpedantic -Wno-unused-variable -Werror -O0 -D_DEFAULT_SOURCE -ggdb3 $(SRCDIR)/test_hamming_decode.c -o test_hamming_decode

test_io:
	gcc -I. -std=c17 -Wall -Wextra -Wno-unused-variable -Werror -O0 -D_DEFAULT_SOURCE -ggdb3 $(SRCDIR)/test_io.c -o test_io

test_process:
	gcc -I. -std=c17 -Wall -Wextra -Wno-unused-variable -Werror -O0 -D_POSIX_C_SOURCE -ggdb3 $(SRCDIR)/test_process.c -o test_process

test_cmd:
	gcc -I. -std=c17 -Wall -Wextra -Wpedantic -Wno-unused-variable -Werror -O0 -D_POSIX_C_SOURCE -ggdb3 $(SRCDIR)/test_cmd.c -o test_cmd

test_profile:
	gcc -I. -std=c17 -Wall -Wextra -Wpedantic -Wno-unused-variable -Werror -O0 -D_POSIX_C_SOURCE -ggdb3 $(SRCDIR)/test_profile.c -o test_profile

test_math:
	gcc -I. -std=c17 -Wall -Wextra -Wpedantic -Wno-unused-variable -Werror -O3 -ggdb3 $(SRCDIR)/test_math.c -o test_math -lm
	gcc -I. -std=c17 -Wall -Wextra -Wpedantic -Wno-unused-variable -Werror -O3 -DHGLM_USE_SIMD -mavx -ggdb3 $(SRCDIR)/test_math.c -o test_math_simd -lm

test_queue:
	gcc -I. -std=c17 -Wall -Wextra -Wno-unused-variable -Werror -O0 -D_POSIX_C_SOURCE -ggdb3 $(SRCDIR)/test_queue.c -o test_queue

test_ini:
	gcc -I. -std=c17 -Wall -Wextra -Wno-unused-variable -Werror -O0 -ggdb3 $(SRCDIR)/test_ini.c -o test_ini

test_worker_pool:
	gcc -I. -std=c17 -Wall -Wextra -Wno-unused-variable -Werror -O0 -ggdb3 -D_POSIX_C_SOURCE=199309L -fsanitize=thread $(SRCDIR)/test_worker_pool.c -o test_worker_pool -lpthread

test_rbtree:
	gcc -I. -std=c17 -Wall -Wextra -Wno-unused-variable -Werror -O0 -ggdb3 $(SRCDIR)/test_rbtree.c -o test_rbtree


clean:
	-rm test_hgl
	-rm test_vec
	-rm test_rbuf
	-rm test_chan
	-rm test_rhashtbl
	-rm test_htable
	-rm test_hset
	-rm test_stack_alloc
	-rm test_arena_alloc
	-rm test_pool_alloc
	-rm test_fs_alloc
	-rm test_hotload
	-rm libmylib.so
	-rm test_serialize
	-rm test_serialize_elfinfo
	-rm test_memdbg
	-rm test_flags
	-rm test_string
	-rm test_fft
	-rm test_hamming
	-rm test_hamming_encode
	-rm test_hamming_decode
	-rm test_io
	-rm test_cmd
	-rm test_process
	-rm test_profile
	-rm test_math
	-rm test_math_simd
	-rm test_queue
	-rm test_ini
	-rm test_worker_pool
	-rm test_rbtree
	-rm testfil_*
