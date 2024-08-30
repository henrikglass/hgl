.PHONY: test_hotload_mylib

MAKEFLAGS := --jobs=$(shell nproc)

SRCDIR := test

all: _test_vec         \
     _test_rbuf        \
     _test_chan        \
     _test_rhashtbl    \
     _test_stack_alloc \
     _test_arena_alloc \
     _test_pool_alloc  \
     _test_hgl         \
     _test_hotload     \
     _test_serialize   \
     _test_memdbg      \
     _test_flags       \
     _test_string      \
     _test_hamming     \
     _test_cmd         \
     _test_io          \
     _test_process     \
     _test_profile     \
     _test_math        \
     _test_fft         \
     _test_ini         \
     _test_queue

_test_hgl:
	gcc -I. -std=c17 -Wall -Wextra -Werror -O0 -ggdb3 -D_POSIX_C_SOURCE=199309L $(SRCDIR)/test_hgl.c -o test_hgl

_test_vec:
	gcc -I. -std=c17 -Wall -Wextra -Wpedantic -Werror -O0 -ggdb3 $(SRCDIR)/test_vec.c -o test_vec

_test_rbuf:
	gcc -I. -std=c17 -Wall -Wextra -Wpedantic -Werror -O0 -ggdb3 $(SRCDIR)/test_rbuf.c -o test_rbuf

_test_chan:
	gcc -I. -std=c17 -Wall -Wextra -Wpedantic -Werror -O0 -ggdb3 $(SRCDIR)/test_chan.c -o test_chan

_test_rhashtbl:
	gcc -I. -std=c17 -Wall -Wextra -Wpedantic -Werror -O0 -ggdb3 $(SRCDIR)/test_rhashtbl.c -o test_rhashtbl

_test_stack_alloc:
	gcc -I. -std=c17 -Wall -Wextra -Wpedantic -Werror -O0 -ggdb3 $(SRCDIR)/test_stack_alloc.c -o test_stack_alloc

_test_arena_alloc:
	gcc -I. -std=c17 -Wall -Wextra -Wpedantic -Werror -O0 -ggdb3 $(SRCDIR)/test_arena_alloc.c -o test_arena_alloc

_test_pool_alloc:
	gcc -I. -std=c17 -Wall -Wextra -Wpedantic -Werror -O0 -ggdb3 $(SRCDIR)/test_pool_alloc.c -o test_pool_alloc

_test_hotload: _test_hotload_mylib
	gcc -I. -D_POSIX_C_SOURCE=200809L -std=c17 -Wall -Wextra -Wpedantic -Werror -O0 -ggdb3 -Wl,-rpath="./" $(SRCDIR)/test_hotload.c -o test_hotload -ldl

_test_hotload_mylib:
	gcc -I. -fPIE -shared -Wall -Wextra -Wpedantic -Wstrict-prototypes -Wmissing-declarations -Wmissing-prototypes -ggdb3 $(SRCDIR)/mylib.c -o libmylib.so

_test_serialize:
	gcc -I. -std=c17 -Wall -Wextra -Wpedantic -Werror -O0 -ggdb3 $(SRCDIR)/test_serialize.c -o test_serialize
	gcc -I. -std=c17 -Wall -Wextra -Wpedantic -Werror -O0 -ggdb3 $(SRCDIR)/test_serialize_elfinfo.c -o test_serialize_elfinfo

_test_memdbg:
	gcc -I. -std=c17 -Wall -Wextra -Wpedantic -Wno-unused-variable -Werror -O0 -ggdb3 $(SRCDIR)/test_memdbg.c -o test_memdbg

_test_flags:
	gcc -I. -std=c17 -Wall -Wextra -Wpedantic -Wno-unused-variable -Werror -O0 -ggdb3 $(SRCDIR)/test_flags.c -o test_flags -lm

_test_string:
	gcc -I. -std=c17 -Wall -Wextra -Wpedantic -Wno-unused-variable -Werror -O0 -ggdb3 $(SRCDIR)/test_string.c -o test_string

_test_fft:
	gcc -I. -std=c17 -Wall -Wextra -Wpedantic -Wno-unused-variable -Werror -O3 -march=native -ffast-math $(SRCDIR)/test_fft.c -o test_fft -lm

_test_hamming:
	gcc -I. -std=c17 -Wall -Wextra -Wpedantic -Wno-unused-variable -Werror -O0 -ggdb3 $(SRCDIR)/test_hamming.c -o test_hamming
	gcc -I. -std=c17 -Wall -Wextra -Wpedantic -Wno-unused-variable -Werror -O0 -D_DEFAULT_SOURCE -ggdb3 $(SRCDIR)/test_hamming_encode.c -o test_hamming_encode
	gcc -I. -std=c17 -Wall -Wextra -Wpedantic -Wno-unused-variable -Werror -O0 -D_DEFAULT_SOURCE -ggdb3 $(SRCDIR)/test_hamming_decode.c -o test_hamming_decode

_test_io:
	gcc -I. -std=c17 -Wall -Wextra -Wno-unused-variable -Werror -O0 -D_DEFAULT_SOURCE -ggdb3 $(SRCDIR)/test_io.c -o test_io

_test_process:
	gcc -I. -std=c17 -Wall -Wextra -Wno-unused-variable -Werror -O0 -D_POSIX_C_SOURCE -ggdb3 $(SRCDIR)/test_process.c -o test_process

_test_cmd:
	gcc -I. -std=c17 -Wall -Wextra -Wpedantic -Wno-unused-variable -Werror -O0 -D_POSIX_C_SOURCE -ggdb3 $(SRCDIR)/test_cmd.c -o test_cmd

_test_profile:
	gcc -I. -std=c17 -Wall -Wextra -Wpedantic -Wno-unused-variable -Werror -O0 -D_POSIX_C_SOURCE -ggdb3 $(SRCDIR)/test_profile.c -o test_profile

_test_math:
	gcc -I. -std=c17 -Wall -Wextra -Wpedantic -Wno-unused-variable -Werror -O3 -ggdb3 $(SRCDIR)/test_math.c -o test_math -lm
	gcc -I. -std=c17 -Wall -Wextra -Wpedantic -Wno-unused-variable -Werror -O3 -DHGLM_USE_SIMD -mavx -ggdb3 $(SRCDIR)/test_math.c -o test_math_simd -lm

_test_queue:
	gcc -I. -std=c17 -Wall -Wextra -Wno-unused-variable -Werror -O0 -D_POSIX_C_SOURCE -ggdb3 $(SRCDIR)/test_queue.c -o test_queue

_test_ini:
	gcc -I. -std=c17 -Wall -Wextra -Wno-unused-variable -Werror -O0 -ggdb3 $(SRCDIR)/test_ini.c -o test_ini

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
	-rm testfil_*
