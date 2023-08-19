import grapes

import pytest


def test_shortest_path_correctness():
    g = grapes.LabeledGraph(is_directed=False)
    g.add_node(1)
    g.add_node(2)
    g.add_node(3)
    g.add_node(4)
    g.add_node(5)

    g.add_edge(1, 2, weight=0.5)
    g.add_edge(1, 3, weight=4.0)
    g.add_edge(2, 3, weight=2.5)
    g.add_edge(4, 5, weight=1.0)

    assert g.shortest_path(1, 1) == [1]
    assert g.shortest_path(4, 5) == [4, 5]
    assert g.shortest_path(5, 4) == [5, 4]
    assert g.shortest_path(1, 2) == [1, 2]
    assert g.shortest_path(1, 3) == [1, 2, 3]
    assert g.shortest_path(1, 4) == []

    g = grapes.LabeledGraph(is_directed=True)
    g.add_node(1)
    g.add_node(2)
    g.add_node(3)
    g.add_node(4)
    g.add_node(5)
    g.add_node(6)

    g.add_edge(1, 3, weight=-2.0)
    g.add_edge(2, 1, weight=4.0)
    g.add_edge(2, 3, weight=3)
    g.add_edge(3, 4, weight=2.0)
    g.add_edge(4, 2, weight=-1.0)
    g.add_edge(5, 6, weight=-1.0)

    assert g.shortest_path(1, 1) == [1]
    assert g.shortest_path(5, 6) == [5, 6]
    assert g.shortest_path(6, 5) == []
    assert g.shortest_path(2, 3) == [
        2,
        1,
        3,
    ]
    assert g.shortest_path(2, 4) == [
        2,
        1,
        3,
        4,
    ]
    assert g.shortest_path(1, 5) == []


def test_shortest_path_errors():
    g = grapes.LabeledGraph()
    g.add_node(1)
    g.add_node(2)
    g.add_edge(1, 2, weight=-1.0)
    with pytest.raises(ValueError):
        g.shortest_path(1, 2, algorithm="dijkstra")
