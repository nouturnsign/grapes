import os

import numpy as np
import pytest

import grapes


def test_labeled():
    g = grapes.LabeledGraph(is_directed=False, is_simple=True)
    g.add_node("A")
    g.add_node("B")
    g.add_node("C")
    g.add_node("D")
    g.add_node("E")

    assert g._underlying_graph.get_nodes() == [0, 1, 2, 3, 4]
    assert g.nodes == ["A", "B", "C", "D", "E"]

    with pytest.raises(grapes.GraphDuplicateNodeError):
        g.add_node("A")

    g.add_edge("A", "B", weight=0.5)
    g.add_edge("A", "C", weight=4.0)
    g.add_edge("B", "C", weight=2.5)
    g.add_edge("D", "E", weight=1.0)

    assert g._underlying_graph.get_edge_count() == 4
    assert g._underlying_graph.get_edges() == [(0, 1), (0, 2), (1, 2), (3, 4)]
    assert g.edges == {
        ("A", "B"): 0.5,
        ("A", "C"): 4.0,
        ("B", "C"): 2.5,
        ("D", "E"): 1.0,
    }

    with pytest.raises(grapes.GraphMissingNodeError):
        g.add_edge("F", "G")

    with pytest.raises(grapes.GraphMissingNodeError):
        g.add_edge("A", "F")

    with pytest.raises(grapes.GraphMissingNodeError):
        g.add_edge("F", "A")

    with pytest.raises(grapes.SimpleGraphWithLoopError):
        g.add_edge("A", "A")

    with pytest.raises(grapes.SimpleGraphWithDuplicateEdgeError):
        g.add_edge("A", "B")

    assert not g.is_connected()
    assert not g.is_bipartite()
    assert sorted(g.get_component_sizes()) == [2, 3]

    g.remove_edge("D", "E")

    assert g._underlying_graph.get_edge_count() == 3
    assert g._underlying_graph.get_edges() == [(0, 1), (0, 2), (1, 2)]
    assert g.edges == {
        ("A", "B"): 0.5,
        ("A", "C"): 4.0,
        ("B", "C"): 2.5,
    }
    assert sorted(g.get_component_sizes()) == [1, 1, 3]

    with pytest.raises(grapes.GraphMissingNodeError):
        g.remove_edge("F", "A")

    with pytest.raises(grapes.GraphMissingNodeError):
        g.remove_edge("A", "G")

    with pytest.raises(grapes.GraphMissingEdgeError):
        g.remove_edge("D", "E")


def test_labeled_conditions():
    g = grapes.LabeledGraph(is_directed=False, is_simple=True)

    g.add_node("A")
    g.add_node("B")
    g.add_node("C")
    g.add_node("D")
    g.add_node("E")

    g.add_edge("A", "D")
    g.add_edge("A", "E")
    g.add_edge("B", "D")
    g.add_edge("B", "E")
    g.add_edge("C", "D")
    g.add_edge("C", "E")

    assert g.is_connected()
    assert g.is_bipartite()


def test_labeled_complete():
    g = grapes.LabeledGraph.complete(labels=["A", "B", "C", "D", "E"])

    assert g.nodes == ["A", "B", "C", "D", "E"]
    assert len(g.edges) == 10

    g = grapes.LabeledGraph.complete(n=100)

    assert len(g.edges) == 4950

    with pytest.raises(ValueError):
        g = grapes.LabeledGraph.complete()
    with pytest.raises(ValueError):
        g = grapes.LabeledGraph.complete(labels=["A", "B"], n=2)


def test_labeled_draw():
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
    g.draw(
        np.array(
            [[0.0, 100.0], [-100.0, 0.0], [100.0, 0.0], [0.0, -100.0]], dtype=np.float32
        ),
        "output.png",
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

    assert os.path.exists("output.png")
    os.remove("output.png")

    g = grapes.LabeledGraph(is_directed=False)
    g.add_node("Alice")
    g.add_node("Bob")
    g.add_node("Carol")
    g.add_node("Dave")

    g.add_edge("Alice", "Carol", weight=-2)
    g.add_edge("Bob", "Alice", weight=4)
    g.add_edge("Bob", "Carol", weight=3)
    g.add_edge("Carol", "Dave", weight=2)
    g.add_edge("Dave", "Bob", weight=-1)
    g.draw(
        g.compute_circular_layout(),
        "output.png",
    )

    assert os.path.exists("output.png")
    os.remove("output.png")
