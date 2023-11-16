MAKEFLAGS := --jobs=$(shell nproc)

all: test_vec test_rbuf test_chan test_rhashtbl test_stack_alloc test_arena_alloc test_neat

test_neat:
	gcc -I../ -std=c17 -Wall -Wextra -Wpedantic -Werror -O0 -ggdb3 test_neat.c -o test_neat

test_vec:
	gcc -I../ -std=c17 -Wall -Wextra -Wpedantic -Werror -O0 -ggdb3 test_vec.c -o test_vec

test_rbuf:
	gcc -I../ -std=c17 -Wall -Wextra -Wpedantic -Werror -O0 -ggdb3 test_rbuf.c -o test_rbuf

test_chan:
	gcc -I../ -std=c17 -Wall -Wextra -Wpedantic -Werror -O0 -ggdb3 test_chan.c -o test_chan

test_rhashtbl:
	gcc -I../ -std=c17 -Wall -Wextra -Wpedantic -Werror -O0 -ggdb3 test_rhashtbl.c -o test_rhashtbl

test_stack_alloc:
	gcc -I../ -std=c17 -Wall -Wextra -Wpedantic -Werror -O0 -ggdb3 test_stack_alloc.c -o test_stack_alloc

test_arena_alloc:
	gcc -I../ -std=c17 -Wall -Wextra -Wpedantic -Werror -O0 -ggdb3 test_arena_alloc.c -o test_arena_alloc

clean:
	-rm test_neat
	-rm test_vec
	-rm test_rbuf
	-rm test_chan
	-rm test_rhashtbl
	-rm test_stack_alloc
	-rm test_arena_alloc
