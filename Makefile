CC=gcc
CFLAGS=-Wall -Wextra -I./librmn/src/PUBLIC_INCLUDES -I./librmn/src
LDFLAGS=-L/home/sbf000/.conda/envs/fst-analysis/lib -lrmn -lm -ldl

test_fst24: test_fst24.c
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)

clean:
	rm -f test_fst24 *.o test.fst test_large.fst 
