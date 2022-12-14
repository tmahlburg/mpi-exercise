#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <mpi.h>

#include "linked_list.h"
#include "matrix.h"

linked_list *bfs_explore(int x, int y, linked_list * explored,
						 int *matrix, int row, int col)
{
	if (is_in_list(explored, x, y)) {
		return explored;
	}

	add_to_list(explored, x, y);
	/*
	 * TOP LEFT
	 */
	if (x > 0 && y > 0 && (matrix[(col * (y - 1)) + (x - 1)])) {
		explored = bfs_explore(x - 1, y - 1, explored, matrix, row, col);
	}
	/*
	 * TOP CENTRE
	 */
	if (y > 0 && (matrix[(col * (y - 1)) + x])) {
		explored = bfs_explore(x, y - 1, explored, matrix, row, col);
	}
	/*
	 * TOP RIGHT
	 */
	if (x < (col - 1) && y > 0 && (matrix[(col * (y - 1)) + (x + 1)])) {
		explored = bfs_explore(x + 1, y - 1, explored, matrix, row, col);
	}
	/*
	 * LEFT CENTRE
	 */
	if (x > 0 && (matrix[(col * y) + (x - 1)])) {
		explored = bfs_explore(x - 1, y, explored, matrix, row, col);
	}
	/*
	 * RIGHT CENTRE
	 */
	if (x < (col - 1) && (matrix[(col * y) + (x + 1)])) {
		explored = bfs_explore(x + 1, y, explored, matrix, row, col);
	}
	/*
	 * BOTTOM LEFT
	 */
	if (x > 0 && y < (row - 1) && (matrix[(col * (y + 1)) + (x - 1)])) {
		explored = bfs_explore(x - 1, y + 1, explored, matrix, row, col);
	}
	/*
	 * BOTTOM CENTRE
	 */
	if (y < (row - 1) && (matrix[(col * (y + 1)) + x])) {
		explored = bfs_explore(x, y + 1, explored, matrix, row, col);
	}
	/*
	 * BOTTOM RIGHT
	 */
	if (x < (col - 1) && y < (row - 1)
		&& (matrix[(col * (y + 1)) + (x + 1)])) {
		explored = bfs_explore(x + 1, y + 1, explored, matrix, row, col);
	}

	return explored;
}

int main(int argc, char *argv[])
{

	/*
	 * mpi init
	 */
	int size, rank;

	MPI_Init(&argc, &argv);
	MPI_Comm_size(MPI_COMM_WORLD, &size);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);

	int root = 0;

	int row, col;
	int *matrix;

	if (rank == root) {
		/*
		 * handle arguments
		 */
		char *file_name;
		if (argc > 1) {
			file_name = argv[1];
		} else {
			file_name = "matrix.txt";
		}
		/*
		 * generate matrix
		 */
		matrix = read_matrix(file_name, &row, &col);
		print_matrix(matrix, row, col);
	}

	/*
	 * broadcat row and col
	 */
	MPI_Bcast(&row, 1, MPI_INT, root, MPI_COMM_WORLD);
	MPI_Bcast(&col, 1, MPI_INT, root, MPI_COMM_WORLD);

	if (rank != root) {
		matrix = allocate_matrix(row, col);
	}

	/*
	 * broadcast matrix
	 */
	MPI_Bcast(matrix, row * col, MPI_INT, root, MPI_COMM_WORLD);

	int *pot_positions_array;
	int scatter_rounds;
	if (rank == root) {
		linked_list *potential_positions;
		int is_first = 1;
		/*
		 * collect potential starting positions
		 */
		for (int i = 0; i < row; i++) {
			int last_cell = 0;
			for (int j = 0; j < col; j++) {
				if (matrix[(col * i) + j] == 1 && last_cell != 1) {
					if (is_first) {
						potential_positions = create_list(j, i);
						is_first = 0;
					} else {
						add_to_list(potential_positions, j, i);
					}
				}
				last_cell = matrix[(col * i) + j];
			}
		}
		int len = get_len_list(potential_positions);
		scatter_rounds = ceil(len * 1.0 / size);

		pot_positions_array = malloc(sizeof(int) * scatter_rounds * size * 2);
		if (!pot_positions_array) {
			perror("malloc failed to allocate pot_positions_array");
			exit(EXIT_FAILURE);
		}
		linked_list *curr = potential_positions;
		/*
		 * prepare array
		 */
		for (int i = 0; curr != NULL; i = i + 2) {
			pot_positions_array[i] = curr->x;
			pot_positions_array[i + 1] = curr->y;
			curr = curr->next;
		}
		free_list(potential_positions);
		for (int i = (len * 2); i < (scatter_rounds * size * 2); i++) {
			pot_positions_array[i] = -1;
		}
	}

	/*
	 * broadcast number of scatters
	 */
	MPI_Bcast(&scatter_rounds, 1, MPI_INT, root, MPI_COMM_WORLD);

	int positions[scatter_rounds][2];
	int j = 0;
	/*
	 * scatter potential starting position
	 */
	for (int i = 0; i < scatter_rounds; i++) {
		MPI_Scatter(&pot_positions_array[i * size * 2], 2, MPI_INT,
					&positions[j], 2, MPI_INT, root, MPI_COMM_WORLD);
		j++;
	}

	/*
	 * {witness x, witness y, size}
	 */
	int result[scatter_rounds][3];
	for (int i = 0; i < scatter_rounds; i++) {
		if (positions[i][0] < 0 && positions[i][1] < 0) {
			result[i][0] = -1;
			result[i][1] = -1;
			result[i][2] = -1;
			continue;
		}
		linked_list *component = create_list(-1, -1);

		component =
			bfs_explore(positions[i][0], positions[i][1], component,
						matrix, row, col);

		int *witness = get_top_left_list(component);
		result[i][0] = witness[0];
		result[i][1] = witness[1];
		free(witness);
		/*
		 * minus one because of the imaginary head pixel [-1, -1]
		 */
		result[i][2] = get_len_list(component) - 1;
		free_list(component);
	}

	int results[scatter_rounds][size][3];
	for (int i = 0; i < scatter_rounds; i++) {
		MPI_Gather(&result[i], 3, MPI_INT, &results[i], 3, MPI_INT,
				   root, MPI_COMM_WORLD);
	}

	if (rank == root) {
		free(pot_positions_array);
		linked_list *printed = create_list(-1, -1);

		printf("\n*** RESULTS ***\n\n");
		for (int i = 0; i < scatter_rounds; i++) {
			for (int j = 0; j < size; j++) {
				/*
				 * make sure not to print anything multiple times
				 */
				if (!is_in_list(printed, results[i][j][0], results[i][j][1])) {
					add_to_list(printed, results[i][j][0], results[i][j][1]);
					printf("Witness: (%d, %d) Size: %d\n", results[i][j][0],
						   results[i][j][1], results[i][j][2]);
				}
			}
		}

		printf("\nTotal number of components: %d\n", get_len_list(printed) - 1);

		free_list(printed);
	}
	free(matrix);

	MPI_Finalize();
}
