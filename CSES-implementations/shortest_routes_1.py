import heapq

n, m = tuple([int(i) for i in input().split()])

visited = dict.fromkeys(range(1,n+1), False)

node_distance = {1: 0}
node_distance.update({i:float('inf') for i in range(2,n+1)})

adjacency_list = {i: {} for i in range(1,n+1)}

for _ in range(m):
    origin, destination, weight = tuple([int(point) for point in input().split()])
    adjacency_list[origin][destination] = min(adjacency_list[origin].get(destination, float('inf')), weight)

next_node_pq = [(0, 1)]

while next_node_pq:
    current_distance, current_node = heapq.heappop(next_node_pq)
    if visited[current_node]:
        continue
    
    visited[current_node] = True
    
    for neighbor, weight in adjacency_list[current_node].items():
        distance = current_distance + weight
        
        if distance < node_distance[neighbor]:
            node_distance[neighbor] = distance
            heapq.heappush(next_node_pq, (distance, neighbor))

print(' '.join(str(int(node_distance[i])) for i in range(1, n+1)))