MAKEFLAGS := --jobs=$(shell nproc)

all: test_vec test_rbuf test_chan test_rhashtbl

test_vec:
	gcc -I../ -Wall -Wextra -Wpedantic -Werror -O0 -ggdb3 test_vec.c -o test_vec

test_rbuf:
	gcc -I../ -Wall -Wextra -Wpedantic -Werror -O0 -ggdb3 test_rbuf.c -o test_rbuf

test_chan:
	gcc -I../ -Wall -Wextra -Wpedantic -Werror -O0 -ggdb3 test_chan.c -o test_chan

test_rhashtbl:
	gcc -I../ -Wall -Wextra -Wpedantic -Werror -O0 -ggdb3 test_rhashtbl.c -o test_rhashtbl

clean:
	-rm test_vec
	-rm test_rbuf
	-rm test_chan
	-rm test_rhashtbl
