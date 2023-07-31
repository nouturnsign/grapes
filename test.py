import grapes

g = grapes.Graph(9)
g.add_edge(0, 5)
g.add_edge(0, 8)
g.add_edge(1, 7)
g.add_edge(2, 5)
g.add_edge(2, 6)
g.add_edge(3, 7)
g.add_edge(3, 8)
g.add_edge(4, 8)

print(g.is_bipartite())
