all: example.png

example.png: example.py
	./$< | dot -Tpng > $@

.PHONY: clean

clean:
	rm -f example.png
