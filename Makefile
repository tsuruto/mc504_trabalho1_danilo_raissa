all: lab1

lab1: lab1.c
	gcc -o lab1 lab1.c -pthread `allegro-config --libs`

clean:
	rm -rf *.o lab1
