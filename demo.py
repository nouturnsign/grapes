import grapes

g = grapes.LabeledGraph(is_directed=True)
g.add_node("A")
g.add_node("B")
g.add_node("C")
g.add_node("D")
g.add_node("E")
g.add_node("F")
g.add_node("G")
g.add_node("H")

g.add_edge("A", "B", weight=7)
g.add_edge("A", "C", weight=9)
g.add_edge("A", "F", weight=14)
g.add_edge("B", "C", weight=10)
g.add_edge("B", "D", weight=15)
g.add_edge("C", "D", weight=11)
g.add_edge("C", "F", weight=2)
g.add_edge("D", "E", weight=6)
g.add_edge("E", "F", weight=9)
g.add_edge("G", "H", weight=1)

print(g.nodes)
print(g.edges)
print(g.shortest_path("A", "E"))
print(g.shortest_path("G", "H"))
print(g.shortest_path("A", "G"))
print(g.is_connected())
print(g.get_component_sizes())
print(g.is_bipartite())
g.draw(
    g.compute_circular_layout(),
    "example.png",
    background_color=grapes.BLACK,
    node_fill_color=grapes.WHITE,
    edge_color=grapes.WHITE,
)
