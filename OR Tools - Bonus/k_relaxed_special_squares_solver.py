from ortools.sat.python import cp_model

def solve_relaxed_squares(k):
    model = cp_model.CpModel()
    
    # top left anchor coordinates
    anchors = [(0, 2), (2, 1), (1, 4), (3, 3)]
    
    cells = set()  # to ensure overlapping cells are not duplicated
    squares = []
    
    for r, c in anchors:
        sq = [(r+i, c+j) for i in range(3) for j in range(3)]
        squares.append(sq)
        cells.update(sq)

    max_val = 500  # large upper bound based on the example where max value used was 137
    grid = {pos: model.NewIntVar(1, max_val, f"v_{pos[0]}_{pos[1]}") for pos in cells}
    
    model.AddAllDifferent(grid.values())

    for sq in squares:
        sums = []
        
        # row sums
        for i in range(3):
            sums.append(sum(grid[sq[i*3+j]] for j in range(3)))
            
        # col sums
        for j in range(3):
            sums.append(sum(grid[sq[i*3+j]] for i in range(3)))
            
        # diag sums
        sums.append(grid[sq[0]]+grid[sq[4]]+grid[sq[8]])
        sums.append(grid[sq[2]]+grid[sq[4]]+grid[sq[6]])

        # diff between any two sums in a box must be <= k
        for s1 in sums:
            for s2 in sums:
                model.Add(s1 - s2 <= k)

    model.Minimize(sum(grid.values()))

    solver = cp_model.CpSolver()
    solver.parameters.num_search_workers = 8 
    
    status = solver.Solve(model)

    if status in [cp_model.OPTIMAL, cp_model.FEASIBLE]:
        print()
        print("Solution for k =", k)
        print()
        print("Minimum Sum:", int(solver.ObjectiveValue()))
        print()
        print("Grid Layout:")
        for r in range(6):
            row_out = []
            for c in range(7):
                if (r, c) in grid:
                    val = solver.Value(grid[(r, c)])
                    row_out.append(f"{val:3}")
                else:
                    row_out.append("   ")
            print(" ".join(row_out))
        print("\n")
    else:
        print("No solution found for k=", k, end='\n')

solve_relaxed_squares(1)
solve_relaxed_squares(7)