NPROC := 0

main: main.c matrix.c matrix.h linked_list.c linked_list.h
	- mpicc -lm -o main main.c matrix.c matrix.h linked_list.c linked_list.h

matrix.txt: gen_matrix.py
	- python3 gen_matrix.py

test_data: matrix.txt

build: main

run: main myhosts
	- mpirun -np $(NPROC) -hosts myhosts main
