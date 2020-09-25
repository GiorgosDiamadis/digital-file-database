all: project3.o objdb.o
	gcc -Wall -g project3.o objdb.o -o all

main.o: project3.c objdb.h
	gcc -Wall -g -c project3.c

util.o: objdb.c objdb.h
	gcc -Wall -g -c objdb.c

