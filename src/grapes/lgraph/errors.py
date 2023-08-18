from typing import Hashable

try:
    from typing import Self
except ImportError:
    from typing_extensions import Self


class GraphMissingNodeError(Exception):
    """Raised when a graph is missing the requested node.

    :param label: Missing node.
    :type label: Hashable
    """

    def __init__(self: Self, label: Hashable) -> None:
        super().__init__(f"Graph is missing the following node: {label=}")


class GraphDuplicateNodeError(Exception):
    """Raised when a graph has a duplicate node.

    :param label: Duplicate node.
    :type label: Hashable
    """

    def __init__(self: Self, label: Hashable) -> None:
        super().__init__(f"Graph has a duplicate of the following node: {label=}")


class SimpleGraphWithLoopError(Exception):
    """Raised when a simple graph contains a self loop.

    :param label: Node that contains self loop.
    :type label: Hashable
    """

    def __init__(self: Self, label: Hashable) -> None:
        super().__init__(f"Simple graph cannot contain a loop. {label=}")


class SimpleGraphWithDuplicateEdgeError(Exception):
    """Raised when a simple graph contains a duplicate edge.

    :param u_label: Node.
    :type u_label: Hashable
    :param v_label: Other node.
    :type v_label: Hashable
    """

    def __init__(self: Self, u_label: Hashable, v_label: Hashable) -> None:
        super().__init__(
            f"Simple graph cannot contain duplicate edges. {u_label=}, {v_label=}"
        )


class RendererInvalidInputError(Exception):
    """Raised when input data ro renderer is incorrectly formatted.

    :param message: Message
    :type message: str
    """

    def __init__(self: Self, message: str) -> None:
        super().__init__(message)
