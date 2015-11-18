all: inflate output

inflate: inflate.c
	gcc -g -o $@ $<

output: output.c
	gcc -o $@ $<


.PHONY: clean

clean:
	rm -f inflate
	rm -f output
