all: weight_test

weight_test: weight_test.o ai_snake.o
	gcc -o weight_test -O3 -std=c99 weight_test.o ai_snake.o

weight_test.o: weight_test.c ai_snake.h
	gcc -c -std=c99 -O3 weight_test.c 

ai_snake.o: ai_snake.c ai_snake.h
	gcc -c -std=c99 -O3 ai_snake.c

clean:
	rm *.o weight_test
