.PHONY: build test translate diff

build:
	clang -g -Wall -std=c2x -o c8086 c8086.c


test: build test-all
#test: build test-dev

test-all: test-decoding test-simulation

test-dev:
	./c8086 ./translated/listing_0046_add_sub_cmp -exec



decode = ./c8086 $(1) | yasm - -o build/$(basename $(notdir $(1))) && cmp -l build/$(basename $(notdir $(1))) $(1);

test-decoding:
	mkdir -p build
	$(foreach file, $(wildcard ./translated/*), $(call decode, $(file)))



simulate = ./c8086 $(1) -exec | diff -w -B -I "---" tests/$(notdir $(1)).txt -

test-simulation:
	$(call simulate, ./translated/listing_0043_immediate_movs)
	$(call simulate, ./translated/listing_0044_register_movs)
	$(call simulate, ./translated/listing_0046_add_sub_cmp)

translate:
	$(foreach file, $(wildcard listings/*.asm), yasm $(file) -o translated/$(basename $(notdir $(file)));)

diff:
	diff --color=always <(xxd ./build/build-41) <(xxd ./translated/listing-41)
