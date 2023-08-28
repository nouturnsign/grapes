"""Provides the underlying C extension.

Contains the Multigraph class, implementing algorithms and data structures
in C.
"""
from typing import Optional

try:
    from typing import Self
except ImportError:
    from typing_extensions import Self

import numpy as np
import numpy.typing as npt

class Multigraph:
    """Underlying graph type.

    :param is_directed: Whether or not the graph is directed.
    :type is_directed: bool
    :param node_count: The initial number of nodes within the graph,
        defaults to 0.
    :type node_count: int

    .. note::
        Nodes are represented as 0-based indices.

    .. warning::
        Exceptions are currently undocumented for this class.
    """

    def __init__(self: Self, is_directed: bool, node_count: int = 0) -> None: ...
    def get_node_count(self: Self) -> int:
        """Get the number of nodes in the graph.

        :rtype: int
        """
    def get_nodes(self: Self) -> list[int]:
        """Get the nodes in the graph.

        :returns: List of nodes
        :rtype: list[int]
        """
    def get_edge_count(self: Self) -> int:
        """Get the number of edges in the graph.

        .. note::
            Edges are considered as having their own identity, so multiple
            edges with the same nodes will be counted separately.

        :rtype: int
        """
    def get_edges(self: Self) -> list[tuple[int, int]]:
        """Get the edges in the graph.

        .. note::
            If the graph is undirected and (u, v) is in edges, (v, u) will not
            also be returned. However, edges are considered as having their own
            identity, so multiple edges will be returned.

        :returns: List of edges
        :rtype: list[tuple[int, int]]
        """
    def get_weights(self: Self) -> list[float]:
        """Get the weights in the graph.

        :returns: List of weights in the same order as edges
        :rtype: list[float]
        """
    def add_node(self: Self) -> int:
        """Add a node to the graph.

        :returns: The index to the new node added.
        :rtype: int
        """
    def add_edge(self: Self, u: int, v: int, *, weight: float = 1.0) -> None:
        """Add an edge between two nodes.

        .. note::
            Edges are considered as having their own identity.

        :param u: node
        :type u: int
        :param v: node
        :type v: int
        :param weight: weight of edge, defaults to 1.0
        :type weight: float
        """
    def remove_edge(self: Self, u: int, v: int) -> None:
        """Remove an existing edge between two nodes.

        :param u: node
        :type u: int
        :param v: node
        :type v: int
        """
    def dijkstra(self: Self, srcs: list[int], dst: int) -> tuple[list[int], list[int]]:
        """Multiple source Dijkstra's algorithm.

        :param srcs: Begin (source) nodes
        :type srcs: list[int]
        :param dst: End (destination) node
        :type dst: int
        :return: Lists of nodes, containing the distances and predecessors.
        :rtype: tuple[list[int], list[int]]
        """
    def bellman_ford(
        self: Self, srcs: list[int], dst: int
    ) -> Optional[tuple[list[int], list[int]]]:
        """Multiple source Bellman-Ford algorithm.

        :param srcs: Begin (source) nodes
        :type srcs: list[int]
        :param dst: End (destination) node
        :type dst: int
        :return: Lists of nodes, containing the distances and predecessors. If
            a negative weight cycle is found, returns `None`.
        :rtype: Optional[tuple[list[int], list[int]]]
        """
    def floyd_warshall(self: Self) -> Optional[tuple[list[list[int]], list[list[int]]]]:
        """Floyd-Warshall algorithm.

        :return: Lists of lists of nodes, containing the distances and
            predecessors. If a negative weight cycle is found, returns `None`.
        :rtype: Optional[tuple[list[list[int]], list[list[int]]]]
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
    def compute_circular_layout(
        self: Self,
        radius: float,
        initial_angle: float,
        x_center: float,
        y_center: float,
    ) -> npt.NDArray[np.float32]:
        """Compute a circular layout for the graph.

        :param radius: Radius of circle
        :type radius: float
        :param initial_angle: Initial angle in radians
        :type initial_angle: float
        :param x_center: x-coordinate of center of circle
        :type x_center: float
        :param y_center: y-coordinate of center of circle
        :type y_center: float
        :returns: (number of nodes) by 2 array describing 2d coordinates
        :rtype: npt.NDArray[np.float32]
        """
