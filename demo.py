import grapes

g = grapes.Graph()
for _ in range(8):
    g.add_node()
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

options = {
    "viewBox_hint": "absolute",
    "viewBox_ul_x": 0,
    "viewBox_ul_y": 0,
    "viewBox_dr_x": 300,
    "viewBox_dr_y": 300,
    "layout_circular_radius": 100,
    "layout_circular_theta0": 1.57079632679,
    "layout_circular_cx": 150,
    "layout_circular_cy": 150,
    "node": {"fill": "white", "label": [chr(i + ord("A")) for i in range(8)]},
}
g.save("out.svg", "svg", "circular", options)
