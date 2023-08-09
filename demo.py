import grapes

g = grapes.Multigraph(False)
for _ in range(8):
    g.add_node()
g.add_edge(0, 1, weight=7)
g.add_edge(0, 2, weight=9)
g.add_edge(0, 5, weight=14)
g.add_edge(1, 2, weight=10)
g.add_edge(1, 3, weight=15)
g.add_edge(2, 3, weight=11)
g.add_edge(2, 5, weight=2)
g.add_edge(3, 4, weight=6)
g.add_edge(4, 5, weight=9)
g.add_edge(6, 7, weight=1)

print(g.get_node_count())
print(g.get_edge_count())
print(g.get_edges())
print(g.dijkstra_path(0, 4))
print(g.dijkstra_path(6, 7))
print(g.dijkstra_path(0, 7))
print(g.get_component_sizes())
print(g.is_bipartite())
