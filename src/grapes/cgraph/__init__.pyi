from typing import Any, Callable, Optional, Union

try:
    from typing import Self
except ImportError:
    from typing_extensions import Self

import os

class Graph:
    """Simple, undirected graph.

    .. note::
        Nodes are represented as 0-based indices.
    """

    def __init__(self: Self, node_count: int = 0) -> None:
        """Initialize a simple, undirected graph.

        :param node_count: The initial number of nodes within the graph, defaults to 0.
        :type node_count: int
        """
    def get_node_count(self: Self) -> int:
        """Get the number of nodes in the graph.

        :rtype: int
        """
    def get_edge_count(self: Self) -> int:
        """Get the number of edges in the graph.

        :rtype: int
        """
    def get_edges(self: Self) -> list[tuple[int, int]]:
        """Get the edges in the graph.

        .. warning::
            If (u, v) is in edges, (v, u) will not also be returned. However,
            duplicate inserted edges will still be duplicated.

        :returns: List of edges
        :rtype: list[tuple[int, int]]
        """
    def add_node(self: Self) -> int:
        """Add a node to the graph.

        :returns: The index to the new node added.
        :rtype: int
        """
    def add_edge(self: Self, u: int, v: int) -> None:
        """Add an undirected edge between two nodes.

        .. warning::
            Adding duplicate edges will not raise an error.

        :param u: node
        :type u: int
        :param v: node
        :type v: int
        :rtype: None
        """
    def dijkstra_path(
        self: Self, src: int, dst: int, weight: Callable[[int, int], float]
    ) -> list[int]:
        """Get the shortest path in the graph using Dijkstra's algorithm.

        .. note::
            The weight function should accept both (u, v) and (v, u) as
            potential inputs.

        :param src: Begin (source) node
        :type src: int
        :param dst: End (destination) node
        :type dst: int
        :param weight: Weight function that accepts two nodes and returns the
            weight as a float.
        :type weight: Callable[[int, int], float]
        :return: List of nodes, starting from `src` and ending with `dst`.
            Returns an empty list if no path found.
        :rtype: list[int]
        """
    def get_component_sizes(self: Self) -> list[int]:
        """Return the sizes of the (connected) components in the graph.

        :rtype: list[int]
        """
    def is_bipartite(self: Self) -> bool:
        """Return whether the graph is bipartite or not.

        :returns: Returns `True` if the graph is bipartite; otherwise, `False`.
        :rtype: bool
        """
