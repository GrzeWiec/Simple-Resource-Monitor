main: main.c 
	gcc -o main main.c 

valgrind: main.out
	valgrind ./main.out

run: main.out
	./main.out

all: main run

vall: main valgrind

clean:
	rm *.out 