#include <stdio.h>
#include <stdlib.h>

#include "matrix.h"

/*
 * Allocates the data structure of matrix, which is
 * an array of ints, where the elements are ordered
 * line after line.
 *
 * int row: amount of rows the matrix is going to
 * have
 * int col: amount of coloumns the matrix is going
 * to have
 *
 * returns int *: the empty matrix as array of ints
 */
int *allocate_matrix(int row, int col)
{
	int *matrix = malloc(row * col * sizeof(int));
	if (!matrix) {
		perror("malloc failed on allocating matrix");
		exit(EXIT_FAILURE);
	}
	return matrix;
}

/* Reads a matrix from a text file following the
 * following format:
 *   Number of rows as int \n
 *   Number of cols as int \n
 *   Matrix as one line of row * cols consecutive
 *     1s and 0s
 * Everything after that is ignored.
 *
 * char *file_name: Name of the file the matrix is
 * in
 * int *row: The number of rows is going to be
 * written here
 * int *col: The number of coloumns is going to be
 * written here
 *
 * returns int *: the read matrix as array of ints
 */
int *read_matrix(char *file_name, int *row, int *col)
{
	FILE *file = fopen(file_name, "r");

	if (!file) {
		perror("couldn't fopen in read_matrix");
		exit(EXIT_FAILURE);
	}

	fscanf(file, "%d\n%d\n", row, col);

	int *matrix = allocate_matrix(*row, *col);

	for (int i = 0; i < *row * *col; i++) {
		char c = fgetc(file);
		if (c == '1' || c == '0') {
			matrix[i] = c - '0';
		} else {
			perror("unexpected character in matrix file");
			exit(EXIT_FAILURE);
		}
	}

	fclose(file);

	return matrix;
}

/*
 * Prints a given matrix as a two dimensional table.
 *
 * int *matrix: the matrix array to print
 * int row: the amount of rows the matrix has
 * int col: the amount of coloumns the matrix has
 */
void print_matrix(int *matrix, int row, int col)
{
	for (int i = 0; i < row; i++) {
		for (int j = 0; j < col; j++) {
			printf("%d ", matrix[(col * i) + j]);
		}
		putchar('\n');
	}
}
