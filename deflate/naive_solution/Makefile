all: example1 example2 example3 inflate

example1: example1.c
	gcc -g -o $@ $<

example2: example2.c
	gcc -g -o $@ $<

example3: example3.c
	gcc -g -o $@ $<

inflate: inflate.c
	gcc -g -o $@ $<

.PHONY: clean

clean:
	rm -f example1
	rm -f example2
	rm -f example3
	rm -f inflate
