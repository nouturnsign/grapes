import grapes

g = grapes.Graph(6)
g.add_edge(0, 1)
g.add_edge(0, 2)
g.add_edge(0, 5)
g.add_edge(1, 2)
g.add_edge(1, 3)
g.add_edge(2, 3)
g.add_edge(2, 5)
g.add_edge(3, 4)
g.add_edge(4, 5)

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
}


def weight(u, v):
    if u > v:
        u, v = v, u
    return weight_dict[(u, v)]


print(g.dijkstra_path(0, 4, weight))
