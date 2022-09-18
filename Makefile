main: main.c matrix.c matrix.h linked_list.c linked_list.h
	- mpicc -lm -o main main.c matrix.c matrix.h linked_list.c linked_list.h

matrix.txt: gen_matrix.py
	- python3 gen_matrix.py

build: main

run: main myhosts matrix.txt
	- mpirun -hostfile myhosts main
