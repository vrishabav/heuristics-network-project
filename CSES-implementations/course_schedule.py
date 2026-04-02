from collections import deque

n, m = tuple([int(i) for i in input().split()])

node_indegree = dict.fromkeys(range(1,n+1), 0)
adjacency_list = {i: [] for i in range(1,n+1)}

for _ in range(m):
    origin, destination = tuple([int(point) for point in input().split()])
    adjacency_list[origin].append(destination)

traversal_order = []
bfs_queue = deque()

for i in range(1,n+1):
    for child in adjacency_list[i]:
        node_indegree[child] += 1

for i in range(1,n+1):
    if node_indegree[i] == 0:
        bfs_queue.append(i)

while bfs_queue:
    node = bfs_queue.popleft()
    traversal_order.append(node)
    for child in adjacency_list[node]:
        node_indegree[child] -= 1
        if node_indegree[child] == 0:
            bfs_queue.append(child)

if len(traversal_order) != n:
    print('IMPOSSIBLE')
else:
    for course in traversal_order:
        print(course, end = ' ')