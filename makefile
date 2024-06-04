.PHONY: test_hotload_mylib

MAKEFLAGS := --jobs=$(shell nproc)

SRCDIR := test

all: test_vec         \
         test_rbuf        \
         test_chan        \
         test_rhashtbl    \
         test_stack_alloc \
         test_arena_alloc \
         test_pool_alloc  \
         test_hgl         \
         test_hotload     \
         test_binpack     \
         test_memdbg      \
         test_flags       \
         test_string      \
         test_hamming     \
         test_cmd         \
         test_io          \
         test_fft

test_hgl:
	gcc -I. -std=c17 -Wall -Wextra -Werror -O0 -ggdb3 $(SRCDIR)/test_hgl.c -o test_hgl

test_vec:
	gcc -I. -std=c17 -Wall -Wextra -Wpedantic -Werror -O0 -ggdb3 $(SRCDIR)/test_vec.c -o test_vec

test_rbuf:
	gcc -I. -std=c17 -Wall -Wextra -Wpedantic -Werror -O0 -ggdb3 $(SRCDIR)/test_rbuf.c -o test_rbuf

test_chan:
	gcc -I. -std=c17 -Wall -Wextra -Wpedantic -Werror -O0 -ggdb3 $(SRCDIR)/test_chan.c -o test_chan

test_rhashtbl:
	gcc -I. -std=c17 -Wall -Wextra -Wpedantic -Werror -O0 -ggdb3 $(SRCDIR)/test_rhashtbl.c -o test_rhashtbl

test_stack_alloc:
	gcc -I. -std=c17 -Wall -Wextra -Wpedantic -Werror -O0 -ggdb3 $(SRCDIR)/test_stack_alloc.c -o test_stack_alloc

test_arena_alloc:
	gcc -I. -std=c17 -Wall -Wextra -Wpedantic -Werror -O0 -ggdb3 $(SRCDIR)/test_arena_alloc.c -o test_arena_alloc

test_pool_alloc:
	gcc -I. -std=c17 -Wall -Wextra -Wpedantic -Werror -O0 -ggdb3 $(SRCDIR)/test_pool_alloc.c -o test_pool_alloc

test_hotload: test_hotload_mylib
	gcc -I. -D_POSIX_C_SOURCE=200809L -std=c17 -Wall -Wextra -Wpedantic -Werror -O0 -ggdb3 -Wl,-rpath="./" $(SRCDIR)/test_hotload.c -o test_hotload -ldl

test_hotload_mylib:
	gcc -I. -fPIE -shared -Wall -Wextra -Wpedantic -Wstrict-prototypes -Wmissing-declarations -Wmissing-prototypes -ggdb3 $(SRCDIR)/mylib.c -o libmylib.so

test_binpack:
	gcc -I. -std=c17 -Wall -Wextra -Wpedantic -Werror -O0 -ggdb3 $(SRCDIR)/test_binpack.c -o test_binpack
	gcc -I. -std=c17 -Wall -Wextra -Wpedantic -Werror -O0 -ggdb3 $(SRCDIR)/test_binpack_elfinfo.c -o test_binpack_elfinfo

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
	gcc -I. -std=c17 -Wall -Wextra -Wpedantic -Wno-unused-variable -Werror -O0 -ggdb3 $(SRCDIR)/test_hamming_encode.c -o test_hamming_encode
	gcc -I. -std=c17 -Wall -Wextra -Wpedantic -Wno-unused-variable -Werror -O0 -ggdb3 $(SRCDIR)/test_hamming_decode.c -o test_hamming_decode

test_io:
	gcc -I. -std=c17 -Wall -Wextra -Wpedantic -Wno-unused-variable -Werror -O0 -D_POSIX_C_SOURCE -ggdb3 $(SRCDIR)/test_io.c -o test_io

test_cmd:
	gcc -I. -std=c17 -Wall -Wextra -Wpedantic -Wno-unused-variable -Werror -O0 -D_POSIX_C_SOURCE -ggdb3 $(SRCDIR)/test_cmd.c -o test_cmd

clean:
	-rm test_hgl
	-rm test_vec
	-rm test_rbuf
	-rm test_chan
	-rm test_rhashtbl
	-rm test_stack_alloc
	-rm test_arena_alloc
	-rm test_pool_alloc
	-rm test_hotload
	-rm libmylib.so
	-rm test_binpack
	-rm test_binpack_elfinfo
	-rm test_memdbg
	-rm test_flags
	-rm test_string
	-rm test_fft
	-rm test_hamming
	-rm test_hamming_encode
	-rm test_hamming_decode
	-rm test_io
	-rm test_cmd
