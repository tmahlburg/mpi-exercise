# /usr/bin/env python3

import random
from typing import List


row = 100
col = 100
# probability a field is 1 and not 0
probability = 0.3
# file name to write the generated matrix to
# used to import the matrix into the mpi program
file_name = 'matrix.txt'


def gen_matrix(row: int, col: int, probability: float) -> List[List[str]]:
    result = []
    for i in range(row):
        line = []
        for j in range(col):
            rand = random.random()
            if rand < probability:
                line.append('1')
            else:
                line.append('0')
        result.append(line)

    return result


def matrix_to_file(matrix: List[List[str]], name: str):
    with open(name, 'w') as f:
        f.write(str(len(matrix)))
        f.write('\n')
        f.write(str(len(matrix[0])))
        f.write('\n')
        for line in matrix:
            f.writelines('\n'.join(line))
            f.write('\n')


def print_matrix(matrix: List[List[str]]):
    for line in matrix:
        for el in line:
            print(el, end=' ')
        print()


def find_component(x: int, y: int, matrix: List[List[str]],
                   explored: List[List[int]]) -> List[List[int]]:
    if [x, y] in explored:
        return explored

    explored.append([x, y])
    row = len(matrix)
    col = len(matrix[0])
    if x > 0 and y > 0:
        if matrix[y-1][x-1] == '1':
            explored = find_component(x-1, y-1, matrix, explored)
    if y > 0:
        if matrix[y-1][x] == '1':
            explored = find_component(x, y-1, matrix, explored)
    if x < (col-1) and y > 0:
        if matrix[y-1][x+1] == '1':
            explored = find_component(x+1, y-1, matrix, explored)
    if x > 0:
        if matrix[y][x-1] == '1':
            explored = find_component(x-1, y, matrix, explored)
    if x < (col-1):
        if matrix[y][x+1] == '1':
            explored = find_component(x+1, y, matrix, explored)
    if x > 0 and y < (row-1):
        if matrix[y+1][x-1] == '1':
            explored = find_component(x-1, y+1, matrix, explored)
    if y < (row-1):
        if matrix[y+1][x] == '1':
            explored = find_component(x, y+1, matrix, explored)
    if x < (col-1) and y < (row-1):
        if matrix[y+1][x+1] == '1':
            explored = find_component(x+1, y+1, matrix, explored)

    return explored


def in_component_list(pixel: List[int],
                      component_list: List[List[List[int]]]):
    for component in component_list:
        if pixel in component:
            return True
    return False


def find_components(matrix: List[List[str]]) -> List[List[List[int]]]:
    result = []
    for i in range(len(matrix)):
        for j in range(len(matrix[0])):
            if matrix[i][j] == '1' and not in_component_list([j, i], result):
                result.append(find_component(j, i, matrix, []))
    for component in result:
        component.sort()
    return result


matrix = gen_matrix(row, col, probability)
matrix_to_file(matrix, file_name)
print_matrix(matrix)
components = find_components(matrix)
print('Components:')
for component in components:
    print(component, 'Size:', len(component))
print('Components counted:', len(components))
