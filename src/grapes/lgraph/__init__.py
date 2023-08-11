from typing import Generic, Hashable, Optional, TypeVar, Union

try:
    from typing import Self
except ImportError:
    from typing_extensions import Self
import numpy.typing as npt

import json
import os
import tempfile
from enum import Enum, auto

import moderngl
import moderngl_window as mglw
import numpy as np

from ..cgraph import Multigraph

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

    def __contains__(self: Self, key: Union[K1, K2]) -> bool:
        return (key in self._original_mapping) or (key in self._inverse_mapping)


class GraphMissingNodeError(Exception):
    """Raised when a graph is missing the requested node.

    :param label: Missing node.
    :type label: Hashable
    """

    def __init__(self, label: Hashable) -> None:
        super().__init__(f"Graph is missing the following node: {label=}")


class GraphDuplicateNodeError(Exception):
    """Raised when a graph has a duplicate node.

    :param label: Duplicate node.
    :type label: Hashable
    """

    def __init__(self, label: Hashable) -> None:
        super().__init__(f"Graph has a duplicate of the following node: {label=}")


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


class GraphWindow(mglw.WindowConfig):
    gl_version = (3, 3)
    title = "Grapes Graph"

    def __init__(self, **kwargs):
        super().__init__(**kwargs)
        mglw.logger.info(
            f"Received node_layout={self.argv.node_layout}, edge_data={self.argv.edge_data}, and config={self.argv.config}"
        )
        with (
            open(self.argv.node_layout, "rb") as node_layout,
            open(self.argv.edge_data, "rb") as edge_data,
            open(self.argv.config, "r") as config,
        ):
            self.node_layout: np.ndarray = np.load(node_layout)
            self.edge_data: np.ndarray = np.load(edge_data)
            self.config: dict = json.load(config)
        if self.argv.delete:
            os.remove(self.argv.node_layout)
            os.remove(self.argv.edge_data)
            os.remove(self.argv.config)

        if self.node_layout.dtype != np.float32:
            raise TypeError(
                f"Node layout should be of type np.float32; got {self.node_layout.dtype}"
            )
        if self.node_layout.ndim != 2 or self.node_layout.shape[1] != 2:
            raise TypeError(
                f"Node layout should be a n x 2 array; got {self.node_layout.shape}"
            )
        if self.edge_data.ndim != 2 or self.edge_data.shape[1] != 2:
            raise TypeError(
                f"Edge layout should be a e x 2 array; got {self.node_layout.shape}"
            )

        mglw.logger.info(f"Successfully loaded node layout, edge layout, and config")
        self.node_layout_flattened = self.node_layout.flatten()

        directory = os.path.dirname(__file__)
        with (
            open(os.path.join(directory, "node.vert"), "r") as node_vertex_shader,
            open(os.path.join(directory, "node.frag"), "r") as node_fragment_shader,
        ):
            self.node_program = self.ctx.program(
                vertex_shader=node_vertex_shader.read(),
                fragment_shader=node_fragment_shader.read(),
            )
        mglw.logger.info(f"Successfully loaded node shaders")
        mglw.logger.info("Got the following internal members from node shaders:")
        for name in self.node_program:
            member = self.node_program[name]
            mglw.logger.info(f"{name} {type(member)} {member}")
        with (
            open(os.path.join(directory, "edge.vert"), "r") as edge_vertex_shader,
            open(os.path.join(directory, "edge.frag"), "r") as edge_fragment_shader,
        ):
            self.edge_program = self.ctx.program(
                vertex_shader=edge_vertex_shader.read(),
                fragment_shader=edge_fragment_shader.read(),
            )
        mglw.logger.info(f"Successfully loaded edge shaders")
        mglw.logger.info("Got the following internal members from edge shaders:")
        for name in self.edge_program:
            member = self.edge_program[name]
            mglw.logger.info(f"{name} {type(member)} {member}")

        # TODO: allow custom node shape
        _theta = np.linspace(0, 2 * np.pi, 360)
        self.node_shape = (0.1 * np.dstack((np.cos(_theta), np.sin(_theta)))).astype(
            np.float32
        )

        # TODO: allow custom view_box
        view_box = (-self.aspect_ratio * 3, -3, self.aspect_ratio * 3, 3)

        left, bottom, right, top = view_box
        z_near = -1
        z_far = 1
        self.camera = np.array(
            [
                [2 / (right - left), 0, 0, 0],
                [0, 2 / (top - bottom), 0, 0],
                [0, 0, -2 / (z_far - z_near), 0],
                [
                    -((right + left) / (right - left)),
                    -((top + bottom) / (top - bottom)),
                    -((z_far + z_near) / (z_far - z_near)),
                    1,
                ],
            ],
            dtype=np.float32,
        )

        if self.config["filled"]:
            self.node_mode = moderngl.TRIANGLE_FAN
        else:
            self.node_mode = moderngl.LINE_STRIP_ADJACENCY

        self.node_offsets = self.node_program["offsets"]
        self.node_mvp = self.node_program["mvp"]
        self.node_instance_vbo = self.ctx.buffer(reserve=self.node_shape.nbytes)
        self.node_vao = self.ctx.simple_vertex_array(
            self.node_program,
            self.node_instance_vbo,
            "in_vert",
        )

        self.edge_mvp = self.edge_program["mvp"]
        self.edge_vbo = self.ctx.buffer(self.node_layout_flattened)
        self.edge_ebo = self.ctx.buffer(self.edge_data)
        self.edge_vao = self.ctx.simple_vertex_array(
            self.edge_program,
            self.edge_vbo,
            "in_vert",
            index_buffer=self.edge_ebo,
            index_element_size=self.edge_data.itemsize,
        )

    @classmethod
    def add_arguments(cls, parser):
        parser.add_argument(
            "--node-layout",
            type=str,
            help="Pass the node layout file (.npy) by path.",
        )

        parser.add_argument(
            "--edge-data",
            type=str,
            help="Pass the edge data file (.npy) by path.",
        )

        parser.add_argument(
            "--config",
            type=str,
            help="Pass the config file (.json) by path.",
        )

        parser.add_argument(
            "--delete",
            action="store_true",
            default=False,
            help="Whether or not to delete the files afterward.",
        )

    def render(self, time, frametime):
        self.ctx.clear(1.0, 1.0, 1.0)
        self.node_offsets.write(self.node_layout_flattened)
        self.node_mvp.write(self.camera)
        self.node_instance_vbo.write(self.node_shape)
        self.node_vao.render(self.node_mode, instances=self.node_layout.shape[0])
        self.edge_mvp.write(self.camera)
        self.edge_vao.render(moderngl.LINES)


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

    def draw(
        self: Self,
        layout: npt.NDArray[np.float32],
        *,
        filled: bool = True,
    ) -> None:
        raw_config = {
            "filled": filled,
        }
        with (
            tempfile.NamedTemporaryFile("w+b", delete=False) as node_layout,
            tempfile.NamedTemporaryFile("w+b", delete=False) as edge_data,
            tempfile.NamedTemporaryFile("w+", delete=False) as config,
        ):
            np.save(node_layout, layout)
            np.save(
                edge_data, np.array(self.underlying_graph.get_edges(), dtype=np.uint32)
            )
            json.dump(raw_config, config)
        mglw.run_window_config(
            GraphWindow,
            args=(
                "--node-layout",
                node_layout.name,
                "--edge-data",
                edge_data.name,
                "--config",
                config.name,
                "--delete",
            ),
        )
