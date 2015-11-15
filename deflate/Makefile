all: inflate output

inflate: inflate.c
	gcc -o $@ $<

output: output.c
	gcc -o $@ $<


.PHONY: clean

clean:
	rm -f inflate
	rm -f output
