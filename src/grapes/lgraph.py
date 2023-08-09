from typing import Generic, Hashable, Optional, TypeVar

try:
    from typing import Self
except ImportError:
    from typing_extensions import Self

from enum import Enum, auto

from .cgraph import Multigraph

K1 = TypeVar("K1", bound=Hashable)
K2 = TypeVar("K2", bound=Hashable)


class ShortestPathAlgorithm(Enum):
    """Implemented shortest path algorithms."""

    DIJKSTRAS = auto()
    """ShortestPathAlgorithm: Dijkstra's algorithm."""
    AUTO = auto()
    """ShortestPathAlgorithm: Automatically choose an algorithm based on 
    preconditions and heuristics.
    """


class InvertibleMapping(Generic[K1, K2]):
    """Invertible dictionary for internal use."""

    def __init__(
        self: Self,
        original: Optional[dict[K1, K2]] = None,
        inverse: Optional[dict[K1, K2]] = None,
        _linked: bool = False,
    ) -> None:
        if original is None:
            self._original_mapping = {}
        else:
            self._original_mapping = original
        if inverse is None:
            self._inverse_mapping = {}
        else:
            self._inverse_mapping = inverse
        if not _linked:
            self.inverse = self.__class__(
                self._inverse_mapping, self._original_mapping, True
            )
            self.inverse.inverse = self

    def __getitem__(self: Self, key: K1) -> K2:
        return self._original_mapping[key]

    def __setitem__(self: Self, key: K1, value: K2) -> None:
        self._original_mapping[key] = value
        self._inverse_mapping[value] = key


class SimpleGraphWithLoopError(Exception):
    """Raised when a simple graph contains a self loop.

    :param label: Node that contains self loop.
    :type label: Hashable
    """

    def __init__(self, label: Hashable) -> None:
        super().__init__(f"Simple graph cannot contain a loop. {label=}")


class SimpleGraphWithDuplicateEdgeError(Exception):
    """Raised when a simple graph contains a duplicate edge.

    :param u_label: Node.
    :type u_label: Hashable
    :param v_label: Other node.
    :type v_label: Hashable
    """

    def __init__(self, u_label: Hashable, v_label: Hashable) -> None:
        super().__init__(
            f"Simple graph cannot contain duplicate edges. {u_label=}, {v_label=}"
        )


class LabeledGraph:
    """Represents a graph, allowing for nodes to be represented by label. The
    class is a thin wrapper for :class:`grapes.Multigraph`.

    :param is_directed: Whether or not the graph is directed, defaults to False
    :type is_directed: bool
    :param is_simple: Whether or not the graph is simple, defaults to True
    :type is_simple: bool
    :param label_data: Optional label data, defaults to None
    :type label_data: InvertibleMapping[Hashable, int]
    :param underlying_graph: Optional :class:`grapes.Multigraph` to
        wrap, defaults to None
    :type underlying_graph: :class:`grapes.Multigraph`
    :param _has_negative_weight: Whether or not the underlying graph has
        edge weights for internal use, defaults to None
    :type _has_negative_weight: bool
    """

    def __init__(
        self: Self,
        is_directed: bool = False,
        is_simple: bool = True,
        label_data: Optional[InvertibleMapping[Hashable, int]] = None,
        underlying_graph: Optional[Multigraph] = None,
        _has_negative_weight: bool = False,
    ) -> None:
        self.is_simple = is_simple
        self.unique_edges = set()
        if label_data is None:
            self.label_data = InvertibleMapping()
        else:
            self.label_data = label_data
        if underlying_graph is None:
            self.underlying_graph = Multigraph(
                is_directed, len(self.label_data._original_mapping.keys())
            )
        else:
            self.underlying_graph = underlying_graph
        self._has_negative_weight = _has_negative_weight

    @property
    def nodes(self: Self) -> list[Hashable]:
        """The nodes in the graph.

        :type: list[Hashable]
        """
        return list(self.label_data._original_mapping.keys())

    @property
    def edges(self: Self) -> list[tuple[Hashable, Hashable]]:
        """The edges in the graph.

        :type: list[tuple[Hashable, Hashable]]
        """
        return [
            (self.label_data.inverse[u], self.label_data.inverse[v])
            for u, v in self.underlying_graph.get_edges()
        ]

    def add_node(self: Self, label: Hashable) -> None:
        """Add a node to the graph.

        :param label: Node
        :type label: Hashable
        """
        self.label_data[label] = self.underlying_graph.add_node()

    def add_edge(
        self: Self,
        u_label: Hashable,
        v_label: Hashable,
        *,
        weight: float = 1.0,
    ) -> None:
        """Add an edge between two nodes.

        :param u_label: Begin (source) node
        :type u_label: Hashable
        :param v_label: End (destination) node
        :type v_label: Hashable
        :param weight: weight of edges, defaults to 1.0
        :type weight: float
        :raises SimpleGraphWithLoopError: Graph is a simple graph and attempted
            to add a self loop.
        :raises SimpleGraphWithDuplicateEdgeError: Graph is a simple graph and
            attempted to add a duplicate edge.
        """
        if self.is_simple:
            if u_label == v_label:
                raise SimpleGraphWithLoopError(u_label)
            elif (u_label, v_label) in self.unique_edges:
                raise SimpleGraphWithDuplicateEdgeError(u_label, v_label)
        self.unique_edges.add((u_label, v_label))
        self.underlying_graph.add_edge(
            self.label_data[u_label],
            self.label_data[v_label],
            weight=weight,
        )
        if weight < 0:
            self._has_negative_weight = True

    def shortest_path(
        self: Self,
        src_label: Hashable,
        dst_label: Hashable,
        algorithm: ShortestPathAlgorithm = ShortestPathAlgorithm.AUTO,
    ) -> list[Hashable]:
        """Get the shortest path in the graph.

        :param src_label: Begin (source) node
        :type src_label: Hashable
        :param dst_label: End (destination) node
        :type dst_label: Hashable
        :param algorithm: Algorithm to use, defaults to
            `ShortestPathAlgorithm.AUTO`
        :type algorithm: :class:`grapes.ShortestPathAlgorithm`
        :raises NotImplementedError: The given algorithm is not implemented.
        :return: List of nodes, starting from `src_label` and ending with
            `dst_label`. Returns an empty list if no path found.
        :rtype: list[Hashable]
        """
        src = self.label_data[src_label]
        dst = self.label_data[dst_label]

        if algorithm == ShortestPathAlgorithm.AUTO:
            if self._has_negative_weight:
                raise NotImplementedError(
                    "Negative weight shortest path algorithm not implemented."
                )
            else:
                algorithm = ShortestPathAlgorithm.DIJKSTRAS

        if algorithm == ShortestPathAlgorithm.DIJKSTRAS:
            path = self.underlying_graph.dijkstra_path(src, dst)
        else:
            raise NotImplementedError(f"{algorithm} not implemented.")
        return [self.label_data.inverse[node] for node in path]

    def get_component_sizes(self: Self) -> list[int]:
        """Return the sizes of the (connected) components in the graph.

        :rtype: list[int]
        """
        return self.underlying_graph.get_component_sizes()

    def is_connected(self: Self) -> bool:
        """Return the whether or not the graph is connected.

        :returns: Returns `True` if the graph is connected; otherwise, `False`.
        :rtype: bool
        """
        return len(self.get_component_sizes()) == 1

    def is_bipartite(self: Self) -> bool:
        """Return whether the graph is bipartite or not.

        :returns: Returns `True` if the graph is bipartite; otherwise, `False`.
        :rtype: bool
        """
        return self.underlying_graph.is_bipartite()
