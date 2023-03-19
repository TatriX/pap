.PHONY: build test translate diff

build:
	clang -g -Wall -std=c2x -o c8086 c8086.c

test: build
	mkdir -p build
	./c8086 ./translated/listing-37 | yasm - -o build/build-37 && cmp -l build/build-37 ./translated/listing-37
	./c8086 ./translated/listing-38 | yasm - -o build/build-38 && cmp -l build/build-38 ./translated/listing-38
	./c8086 ./translated/listing-39 | yasm - -o build/build-39 && cmp -l build/build-39 ./translated/listing-39
	./c8086 ./translated/listing-40 | yasm - -o build/build-40 && cmp -l build/build-40 ./translated/listing-40
	./c8086 ./translated/listing-41 | yasm - -o build/build-41 && cmp -l build/build-41 ./translated/listing-41

translate:
	yasm listing-37.asm -o translated/listing-37
	yasm listing-38.asm -o translated/listing-38
	yasm listing-39.asm -o translated/listing-39
	yasm listing-40.asm -o translated/listing-40
	yasm listing-41.asm -o translated/listing-41


diff:
	diff --color=always <(xxd ./build/build-41) <(xxd ./translated/listing-41)
