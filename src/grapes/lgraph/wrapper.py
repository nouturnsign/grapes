from typing import Hashable, Optional, Union

try:
    from typing import Self
except ImportError:
    from typing_extensions import Self
import numpy.typing as npt

import json
import tempfile
from enum import Enum, auto

import moderngl_window as mglw
import numpy as np

from .errors import (
    GraphDuplicateNodeError,
    GraphMissingNodeError,
    SimpleGraphWithDuplicateEdgeError,
    SimpleGraphWithLoopError,
)
from .invmap import InvertibleMapping
from .renderer import GraphWindow
from ..cgraph import Multigraph


class ShortestPathAlgorithm(Enum):
    """Implemented shortest path algorithms."""

    DIJKSTRAS = auto()
    """ShortestPathAlgorithm: Dijkstra's algorithm."""
    AUTO = auto()
    """ShortestPathAlgorithm: Automatically choose an algorithm based on 
    preconditions and heuristics.
    """


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
    def edges(self: Self) -> dict[tuple[Hashable, Hashable], float]:
        """The edges in the graph with their corresponding weight.

        :type: dict[tuple[Hashable, Hashable], float]
        """
        return {
            (self.label_data.inverse[u], self.label_data.inverse[v]): w
            for (u, v), w in zip(
                self.underlying_graph.get_edges(), self.underlying_graph.get_weights()
            )
        }

    def add_node(self: Self, label: Hashable) -> None:
        """Add a node to the graph.

        :param label: Node
        :type label: Hashable
        :raises GraphDuplicateNodeError: Graph already contains the given node.
        """
        if label in self.label_data:
            raise GraphDuplicateNodeError(label)
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
        :raises GraphMissingNodeError: Graph is missing one of the nodes.
        :raises SimpleGraphWithLoopError: Graph is a simple graph and attempted
            to add a self loop.
        :raises SimpleGraphWithDuplicateEdgeError: Graph is a simple graph and
            attempted to add a duplicate edge.
        """
        if u_label not in self.label_data:
            raise GraphMissingNodeError(u_label)
        if v_label not in self.label_data:
            raise GraphMissingNodeError(v_label)
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
        :raises GraphMissingNodeError: Graph is missing one of the nodes.
        :raises NotImplementedError: The given algorithm is not implemented.
        :return: List of nodes, starting from `src_label` and ending with
            `dst_label`. Returns an empty list if no path found.
        :rtype: list[Hashable]
        """
        if src_label not in self.label_data:
            raise GraphMissingNodeError(src_label)
        if dst_label not in self.label_data:
            raise GraphMissingNodeError(dst_label)
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
        return self.underlying_graph.compute_circular_layout(
            radius, initial_angle, x_center, y_center
        )

    def draw(
        self: Self,
        layout: npt.NDArray[np.float32],
        *,
        filled: bool = True,
    ) -> None:
        """Draw the graph.

        :param layout: A n x 2 array with dtype np.float32 representing the 2d
            coordinates of each node.
        :type layout: npt.NDArray[np.float32]
        :param filled: Whether or not to fill node shape, defaults to True
        :type filled: bool

        .. note::
            Currently, exceptions are undocumented.
        """
        raw_config = {
            "filled": filled,
        }
        with (
            tempfile.NamedTemporaryFile("w+b", delete=False) as node_layout,
            tempfile.NamedTemporaryFile("w+b", delete=False) as edge_data,
            tempfile.NamedTemporaryFile("w+b", delete=False) as weight_data,
            tempfile.NamedTemporaryFile("w+", delete=False) as config,
        ):
            np.save(node_layout, layout)
            np.save(
                edge_data, np.array(self.underlying_graph.get_edges(), dtype=np.uint32)
            )
            np.save(
                weight_data,
                np.array(self.underlying_graph.get_weights(), dtype=np.float32),
            )
            json.dump(raw_config, config)
        mglw.run_window_config(
            GraphWindow,
            args=(
                "--node-layout",
                node_layout.name,
                "--edge-data",
                edge_data.name,
                "--weight-data",
                weight_data.name,
                "--config",
                config.name,
                "--delete",
            ),
        )
