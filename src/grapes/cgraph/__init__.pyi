from typing import Callable

try:
    from typing import Self
except ImportError:
    from typing_extensions import Self

class Graph:
    """Simple, undirected graph.

    .. note::
        Nodes are represented as 0-based indices.
    """

    def __init__(self: Self, node_count: int) -> None:
        """Initialize a simple, undirected graph.

        :param node_count: The initial number of nodes within the graph.
        :type node_count: int
        """
    def get_node_count(self: Self) -> int:
        """Get the number of nodes in the graph.

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

        :param src: Begin (source) node
        :type src: int
        :param dst: End (destination) node
        :type dst: int
        :param weight: Weight function that accepts two integers and returns the
            weight as a float.
        :type weight: Callable[[int, int], float]
        :return: List of nodes, starting from `src` and ending with `dst`.
            Returns an empty list if no path found.
        :rtype: list[int]
        """
    def kruskal_mst(
        self: Self, weight: Callable[[int, int], float]
    ) -> list[tuple[int, int]]:
        """Get the minimum spanning tree as a list of edges.

        :param weight: Weight function that accepts two integers and returns the
            weight as a float.
        :type weight: Callable[[int, int], float]
        :return: List of edges
        :rtype: list[tuple[int, int]]
        """
