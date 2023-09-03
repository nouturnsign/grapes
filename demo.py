import numpy as np

import grapes

g = grapes.LabeledGraph(is_directed=True)
g.add_node("1")
g.add_node("2")
g.add_node("3")
g.add_node("4")

g.add_edge("1", "3", weight=-2)
g.add_edge("2", "1", weight=4)
g.add_edge("2", "3", weight=3)
g.add_edge("3", "4", weight=2)
g.add_edge("4", "2", weight=-1)

print(g.nodes)
print(g.edges)
print(g.get_outdegree("2"))
print(g.shortest_path("4", "3"))
print(g.shortest_path("2", "3"))
print(g.shortest_path("2", "4"))
print(g.is_connected())
print(g.get_component_sizes())
print(g.is_bipartite())
g.draw(
    np.array(
        [[0.0, 100.0], [-100.0, 0.0], [100.0, 0.0], [0.0, -100.0]], dtype=np.float32
    ),
    background_color=grapes.colors.BLACK,
    node_radius=20.0,
    node_fill_color=grapes.colors.WHITE,
    edge_segment_width=1.0,
    edge_arrowhead_height=15.0,
    edge_arrowhead_width=5.0,
    edge_color=grapes.colors.WHITE,
    has_labels=True,
    label_font_size=30.0,
    label_font_color=grapes.colors.BLACK,
)
