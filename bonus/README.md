Here I have used Google OR-Tools (CP-SAT solver)

#### Explanation
The shape is mapped to a 6x7 grid, and the four squares are defined by their top-left anchors as directed to (0, 2), (2, 1), (1, 4), and (3, 3). Adding these generated coordinates to a Python set automatically deduplicates the overlapping cells, yielding exactly 28 variables.
Each of the cells is a strictly positive integer variable (>= 1). The upper bound is capped at 500 based on the example case where the max value used was 137. An `AllDifferent` constraint is applied globally
For each 3x3 square, the 8 relevant sums (3 rows, 3 columns, 2 diagonals) are calculated. A loop enforces the condition for absolute difference between any two combinations to be <= k
Then we set the solver's objective to minimize the sum of all 28 variables (squares)
