import grapes

g = grapes.Graph(8)
g.add_edge(0, 1)
g.add_edge(0, 2)
g.add_edge(0, 5)
g.add_edge(1, 2)
g.add_edge(1, 3)
g.add_edge(2, 3)
g.add_edge(2, 5)
g.add_edge(3, 4)
g.add_edge(4, 5)
g.add_edge(6, 7)

weight_dict = {
    (0, 1): 7,
    (0, 2): 9,
    (0, 5): 14,
    (1, 2): 10,
    (1, 3): 15,
    (2, 3): 11,
    (2, 5): 2,
    (3, 4): 6,
    (4, 5): 9,
    (6, 7): 1,
}


def weight(u, v):
    if u > v:
        u, v = v, u
    return weight_dict[(u, v)]


print(g.get_node_count())
print(g.get_edge_count())
print(g.get_edges())
print(g.dijkstra_path(0, 4, weight))
print(g.dijkstra_path(6, 7, weight))
print(g.dijkstra_path(0, 7, weight))
print(g.get_component_sizes())
print(g.is_bipartite())
g.save("out.svg", "svg", "circular")
