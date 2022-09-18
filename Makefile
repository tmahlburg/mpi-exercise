main: main.c matrix.c matrix.h linked_list.c linked_list.h
	- mpicc -lm -o main main.c matrix.c matrix.h linked_list.c linked_list.h

build: main

run: main myhosts
	- mpirun -hostfile myhosts main
