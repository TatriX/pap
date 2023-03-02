build:
	clang -g -Wall -std=c2x -o c8086 c8086.c


test: build
	./c8086 ./translated/listing-37 | diff -BI';.*' listing-37.asm -
	./c8086 ./translated/listing-38 | diff -BI';.*' listing-38.asm -

translate:
	yasm listing-37.asm -o translated/listing-37
	yasm listing-38.asm -o translated/listing-38
