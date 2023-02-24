all:
	gcc -g -Wall -std=c2x -o haversine haversine.c -lm # -DDEBUG

gen:
	gcc -g -Wall -std=c2x -o haversine_gen haversine_gen.c

