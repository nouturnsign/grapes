import pytest

import grapes


def test_bellman_ford_correctness():
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

    assert g.shortest_path(1, 1, grapes.ShortestPathAlgorithm.BELLMAN_FORD) == [1]
    assert g.shortest_path(5, 6, grapes.ShortestPathAlgorithm.BELLMAN_FORD) == [5, 6]
    assert g.shortest_path(6, 5, grapes.ShortestPathAlgorithm.BELLMAN_FORD) == []
    assert g.shortest_path(2, 3, grapes.ShortestPathAlgorithm.BELLMAN_FORD) == [
        2,
        1,
        3,
    ]
    assert g.shortest_path(2, 4, grapes.ShortestPathAlgorithm.BELLMAN_FORD) == [
        2,
        1,
        3,
        4,
    ]
    assert g.shortest_path(1, 5, grapes.ShortestPathAlgorithm.BELLMAN_FORD) == []


def test_bellman_ford_errors():
    g = grapes.LabeledGraph()
    with pytest.raises(grapes.GraphMissingNodeError):
        g.shortest_path(1, 2, grapes.ShortestPathAlgorithm.BELLMAN_FORD)
    g.add_node(1)
    with pytest.raises(grapes.GraphMissingNodeError):
        g.shortest_path(1, 2, grapes.ShortestPathAlgorithm.BELLMAN_FORD)
    g.add_node(2)
    g.add_edge(1, 2, weight=-1.0)
    with pytest.raises(grapes.NegativeCycleError):
        g.shortest_path(1, 2, grapes.ShortestPathAlgorithm.BELLMAN_FORD)
