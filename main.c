#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <mpi.h>

#include "linked_list.h"

int * allocate_matrix(int row, int col) {
	int * matrix = malloc(row * col * sizeof(int));
	if (!matrix) {
		perror("malloc failed");
		exit(EXIT_FAILURE);
	}
	return matrix;
}

void generate_matrix(int * matrix, int row, int col) {
	srand(time(NULL));

	for (int i = 0; i < row; i++) {
		for (int j = 0; j < col; j++) {
			matrix[i + (row * j)] = rand() % 2;
		}
	}
}

void print_matrix(int * const matrix, int row, int col) {
	for (int i = 0; i < row; i++) {
		for (int j = 0; j < col; j++) {
			printf("%d ", matrix[i + (row * j)]);
		}
		putchar('\n');
	}
}

linked_list * bfs_explore(int x, int y, linked_list * explored,
						  int * matrix, int row, int col) {
	if (is_in_list(explored, x, y)) {
		return explored;
	}

    add_to_list(explored, x, y);
	/* TOP LEFT */
	if (x > 0 && y > 0 && (matrix[(x - 1) + (row * (y - 1))] == 1)) {
		explored = bfs_explore(x - 1, y - 1, explored, matrix, row, col);
	}
	/* TOP CENTRE */
	if (y > 0 && (matrix[x + (row * (y - 1))])) {
		explored = bfs_explore(x, y - 1, explored, matrix, row, col);
	}
	/* TOP RIGHT */
	if (x < col && y > 0 && (matrix[(x + 1) + (row * (y - 1))])) {
		explored = bfs_explore(x + 1, y - 1, explored, matrix, row, col);
	}
	/* LEFT CENTRE */
	if (x > 0 && (matrix[(x - 1) + (row * y)] == 1)) {
		explored = bfs_explore(x - 1, y - 1, explored, matrix, row, col);
	}
	/* RIGHT CENTRE */
	if (x < col && (matrix[(x + 1) + (row * y)] == 1)) {
		explored = bfs_explore(x - 1, y - 1, explored, matrix, row, col);
	}
	/* BOTTOM LEFT */
	if (x > 0 && y < row && (matrix[(x - 1) + (row * (y + 1))] == 1)) {
		explored = bfs_explore(x - 1, y - 1, explored, matrix, row, col);
	}
	/* BOTTOM CENTRE */
	if (y < row && (matrix[x + (row * (y + 1))] == 1)) {
		explored = bfs_explore(x - 1, y - 1, explored, matrix, row, col);
	}
	/* BOTTOM RIGHT */
	if (x < col && y < row && (matrix[(x + 1) + (row * (y + 1))] == 1)) {
		explored = bfs_explore(x - 1, y - 1, explored, matrix, row, col);
	}

	return explored;
}

int main(int argc, char *argv[]) {
	/* mpi init */
	int size, rank, namelen, root;
	char processor_name[MPI_MAX_PROCESSOR_NAME];

	MPI_Init(&argc, &argv);
	MPI_Comm_size(MPI_COMM_WORLD, &size);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Get_processor_name(processor_name, &namelen);

	int row = 10;
	int col = 20;
	int * matrix = allocate_matrix(row, col);

	root = 0;

	if (rank == root) {
		/* generate matrix */
		generate_matrix(matrix, row, col);
		print_matrix(matrix, row, col);
	}

	/* broadcast matrix */
	MPI_Bcast(matrix, row * col, MPI_INT, root, MPI_COMM_WORLD);

	linked_list * potential_positions;
	int * pot_positions_array;
	int scatter_rounds;
	if (rank == root) {
		int is_first = 1;
		/* collect potential starting positions */
		for (int i = 0; i < row; i++) {
			int last_cell = 0;
			for (int j = 0; j < col; j++) {
				if (matrix[i + (row * j)] == 1 && last_cell != 1) {
					if (is_first) {
						potential_positions = create_list(i, j);
						is_first = 0;
					} else {
						add_to_list(potential_positions, i, j);
					}
				}
				last_cell = matrix[i + (row * j)];
			}
		}
		int len = get_len_list(potential_positions);
		scatter_rounds = ceil(len / size);

		pot_positions_array = malloc(sizeof(int) * (len + (len % size)) * 2);
		linked_list * curr = potential_positions;
		/* prepare array */
		for (int i = 0; curr->next != NULL; i = i + 2) {
			pot_positions_array[i] = curr->x;
			pot_positions_array[i + 1] = curr->y;
			curr = curr->next;
		}
		free_list(potential_positions);
		for (int i = (len * 2); i < ((len + (len % size)) * 2); i++) {
			pot_positions_array[i] = -1;
		}
	}

	/* broadcast number of scatters */
	MPI_Bcast(&scatter_rounds, 1, MPI_INT, root, MPI_COMM_WORLD);

	int positions[scatter_rounds][2];
	int j = 0;
	/* scatter potential starting position */
	for (int i = 0; i < scatter_rounds; i = i + size) {
		MPI_Scatter(&pot_positions_array[i * 2], 2, MPI_INT, &positions[j], 2, MPI_INT, root, MPI_COMM_WORLD);
		j++;
	}
	free(pot_positions_array);

	/* {witness x, witness y, size} */
	int result[scatter_rounds][3];
	for (int i = 0; i < scatter_rounds; i++) {
		linked_list * component = create_list(-1, -1);

		component = bfs_explore(positions[i][0], positions[i][1], component,
								matrix, row, col);

		int * witness = NULL;
		get_top_left_list(component, witness);
		result[i][0] = witness[0];
		result[i][1] = witness[1];
		free(witness);
		/* minus one because of the imaginary head pixel [-1, -1] */
		result[i][2] = get_len_list(component) - 1;
		free_list(component);
	}

	int results[scatter_rounds][size][3];
	for (int i = 0; i < scatter_rounds; i++) {
		MPI_Gather(&result[i], 3, MPI_INT, &results[i], 3, MPI_INT, root, MPI_COMM_WORLD);
	}

	/* root: process results
	 *  -> doppelte ergebnisse löschen
	 */

	/* root: print results
	 *  -> Anzahl Ergebnisse
	 *  -> je Ergebnis: Zeugenpixel.x, Zeugenpixel.y, Anzahl zugehöriger Pixel
	 */

	free(matrix);

	MPI_Finalize();
}
