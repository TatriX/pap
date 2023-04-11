.PHONY: build test translate diff

build:
	clang -g -Wall -std=c2x -o c8086 c8086.c

test: build
	mkdir -p build
	$(foreach file, $(wildcard ./translated/*), ./c8086 $(file) | yasm - -o build/$(basename $(notdir $(file))) && cmp -l build/$(basename $(notdir $(file))) $(file);)


translate:
	$(foreach file, $(wildcard listings/*.asm), yasm $(file) -o translated/$(basename $(notdir $(file)));)

diff:
	diff --color=always <(xxd ./build/build-41) <(xxd ./translated/listing-41)
