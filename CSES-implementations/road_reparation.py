n, m = tuple([int(i) for i in input().split()])

roads = []
for _ in range(m):
    origin, destination, weight = tuple([int(point) for point in input().split()])
    roads.append((weight, origin, destination))

roads.sort()
parent = list(range(n + 1))
size = [1] * (n + 1)

def find(v):
    w = v
    while parent[w] != w:
        parent[w] = parent[parent[w]]
        w = parent[w]
    return w

def union(v1, v2):
    root1 = find(v1)
    root2 = find(v2)

    if root1 == root2:
        return False

    if size[root1] < size[root2]:
        parent[root1] = root2
        size[root2] += size[root1]
    else:
        parent[root2] = root1
        size[root1] += size[root2]
        
    return True

road_cost = 0
edges_added = 0

if n == 1:
    print(0)
else:
    for edge in roads:
        
        if union(edge[1], edge[2]):
            road_cost += edge[0]
            edges_added += 1
            if edges_added == n-1:
                print(road_cost)
                break
    else:
        print('IMPOSSIBLE')