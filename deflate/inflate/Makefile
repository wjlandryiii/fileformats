all: test

inflate.o: inflate.c
	gcc -g -c -o $@ $<

test: test.c inflate.o
	gcc -g -o $@ $^

.PHONY: clean

clean:
	rm -f *.o
	rm -f test
