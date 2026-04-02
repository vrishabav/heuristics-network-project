from collections import deque
 
n, m = tuple([int(i) for i in input().split()])
 
visited = dict.fromkeys(range(1,n+1), False)
adjacency_list = {i: [] for i in range(1,n+1)}
 
for _ in range(m):
    point_1, point_2 = tuple([int(point) for point in input().split()])
    if point_2 not in adjacency_list[point_1]:
        adjacency_list[point_1].append(point_2)
        adjacency_list[point_2].append(point_1)
    
cluster_count = 0
cluster_header = []
 
for city in range(1,n+1):
    if not visited[city]:
        bfs_queue = deque([city])
        cluster_header.append(city)
        
        while len(bfs_queue) > 0:
            node = bfs_queue.popleft()
            
            for neighbour in adjacency_list[node]:
                if not visited[neighbour]:
                    bfs_queue.append(neighbour)
            
            visited[node] = True
 
        cluster_count += 1
 
print(cluster_count-1)
for idx in range(len(cluster_header)-1):
    print(cluster_header[idx], cluster_header[idx+1])
