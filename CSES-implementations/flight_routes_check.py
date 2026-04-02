from collections import deque
 
n, m = tuple([int(i) for i in input().split()])
 
visited = dict.fromkeys(range(1,n+1), False)
adjacency_list = {i: [] for i in range(1,n+1)}
reverse_adjacency_list = {i: [] for i in range(1,n+1)}
 
for _ in range(m):
    origin, destination = tuple([int(point) for point in input().split()])
    adjacency_list[origin].append(destination)
    reverse_adjacency_list[destination].append(origin)

cluster_header = []
 
for city in range(1,n+1):
    if not visited[city]:
        visited[city] = True
        bfs_queue = deque([city])
        cluster_header.append(city)
        
        while len(bfs_queue) > 0:
            node = bfs_queue.popleft()
            
            for neighbour in adjacency_list[node]:
                if not visited[neighbour]:
                    visited[neighbour] = True
                    bfs_queue.append(neighbour)

if len(cluster_header) > 1:
    print('NO')
    print(cluster_header[0], cluster_header[1])
else:
    visited = dict.fromkeys(range(1,n+1), False)
    cluster_header2 = []
    
    for city in range(1,n+1):
        if not visited[city]:
            visited[city] = True
            bfs_queue = deque([city])
            cluster_header2.append(city)
            
            while len(bfs_queue) > 0:
                node = bfs_queue.popleft()
                
                for neighbour in reverse_adjacency_list[node]:
                    if not visited[neighbour]:
                        visited[neighbour] = True
                        bfs_queue.append(neighbour)

    if len(cluster_header2) > 1:
        print('NO')
        print(cluster_header2[1], cluster_header2[0])
    else:
        print('YES')