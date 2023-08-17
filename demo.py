import grapes

g = grapes.LabeledGraph(is_directed=True)
g.add_node("Alice")
g.add_node("Bob")
g.add_node("Carol")
g.add_node("Dave")
g.add_node("E")
g.add_node("F")
g.add_node("G")
g.add_node("H")

g.add_edge("Alice", "Bob", weight=7)
g.add_edge("Alice", "Carol", weight=9)
g.add_edge("Alice", "F", weight=14)
g.add_edge("Bob", "Carol", weight=10)
g.add_edge("Bob", "Dave", weight=15)
g.add_edge("Carol", "Dave", weight=11)
g.add_edge("Carol", "F", weight=2)
g.add_edge("Dave", "E", weight=6)
g.add_edge("E", "F", weight=9)
g.add_edge("G", "H", weight=1)

print(g.nodes)
print(g.edges)
print(g.shortest_path("Alice", "E"))
print(g.shortest_path("G", "H"))
print(g.shortest_path("Alice", "G"))
print(g.is_connected())
print(g.get_component_sizes())
print(g.is_bipartite())
g.draw(
    g.compute_circular_layout(),
    background_color=grapes.BLACK,
    node_fill_color=grapes.WHITE,
    edge_color=grapes.WHITE,
    has_labels=True,
    label_font_size=50.0,
    label_font_color=grapes.TABLEAU_ORANGE,
)
