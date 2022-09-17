#ifndef MATRIX_H
#define MATRIX_H

extern int *allocate_matrix(int row, int col);
extern int *read_matrix(char *file_name, int *row, int *col);
extern void print_matrix(int *matrix, int row, int col);

#endif							/* MATRIX_H */
