import pytest

import grapes


def test_dijkstras_correctness():
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

    assert g.shortest_path(1, 1, grapes.ShortestPathAlgorithm.DIJKSTRAS) == [1]
    assert g.shortest_path(4, 5, grapes.ShortestPathAlgorithm.DIJKSTRAS) == [4, 5]
    assert g.shortest_path(5, 4, grapes.ShortestPathAlgorithm.DIJKSTRAS) == [5, 4]
    assert g.shortest_path(1, 2, grapes.ShortestPathAlgorithm.DIJKSTRAS) == [1, 2]
    assert g.shortest_path(1, 3, grapes.ShortestPathAlgorithm.DIJKSTRAS) == [1, 2, 3]
    assert g.shortest_path(1, 4, grapes.ShortestPathAlgorithm.DIJKSTRAS) == []

    g.remove_edge(4, 5)
    assert g.shortest_path(4, 5, grapes.ShortestPathAlgorithm.DIJKSTRAS) == []

    g.remove_node(2)
    assert g.shortest_path(1, 3, grapes.ShortestPathAlgorithm.DIJKSTRAS) == [1, 3]


def test_dijkstras_errors():
    g = grapes.LabeledGraph()
    with pytest.raises(grapes.GraphMissingNodeError):
        g.shortest_path(1, 2, grapes.ShortestPathAlgorithm.DIJKSTRAS)
    g.add_node(1)
    with pytest.raises(grapes.GraphMissingNodeError):
        g.shortest_path(1, 2, grapes.ShortestPathAlgorithm.DIJKSTRAS)
    g.add_node(2)
    g.add_edge(1, 2, weight=-1.0)
    with pytest.raises(grapes.AlgorithmPreconditionError):
        g.shortest_path(1, 2, grapes.ShortestPathAlgorithm.DIJKSTRAS)
