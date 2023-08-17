from typing import Any

try:
    from typing import Self
except ImportError:
    from typing_extensions import Self
import numpy.typing as npt

import json
import logging
import os

import moderngl
import moderngl_window as mglw
import numpy as np
import pyrr
from PIL import Image

from .errors import RendererInvalidInputError

Image.MAX_IMAGE_PIXELS = 95 * 1238 * 2048
mglw.setup_basic_logging(logging.WARNING)


class RendererConfig(dict):
    def __init__(self: Self, underlying_config: dict) -> None:
        super().__init__(underlying_config)

    def __getitem__(self: Self, __key: Any) -> Any:
        if __key not in self:
            raise RendererInvalidInputError(f"Config is missing key={__key}")
        return super().__getitem__(__key)


class GrapesRenderer(mglw.WindowConfig):
    """Grapes's graph visualization renderer."""

    gl_version = (3, 3)
    """gl_version: Minimum version of 3.3."""
    title = "grapes-graph"
    """title: Window title set to \"grapes-graph\""""

    def __init__(self: Self, **kwargs):
        super().__init__(**kwargs)
        mglw.logger.info(
            f"Received node_layout={self.argv.node_layout}, "
            f"edge_data={self.argv.edge_data}, "
            f"weight_data={self.argv.weight_data}, "
            f"label_data={self.argv.label_data}, "
            f"config={self.argv.config}, "
            f"save_path={self.argv.save_path}"
        )
        with (
            open(self.argv.node_layout, "rb") as node_layout,
            open(self.argv.edge_data, "rb") as edge_data,
            open(self.argv.weight_data, "rb") as weight_data,
            open(self.argv.label_data, "rb") as label_data,
            open(self.argv.config, "r") as config,
        ):
            self.node_layout: np.ndarray = np.load(node_layout)
            self.edge_data: np.ndarray = np.load(edge_data)
            self.weight_data: np.ndarray = np.load(weight_data)
            self.label_data: np.ndarray = np.load(label_data)
            self.config: dict = RendererConfig(json.load(config))
        if self.argv.delete:
            os.remove(self.argv.node_layout)
            os.remove(self.argv.edge_data)
            os.remove(self.argv.weight_data)
            os.remove(self.argv.label_data)
            os.remove(self.argv.config)
        self.save_path = self.argv.save_path
        self.has_edges = self.edge_data.size > 0

        if self.node_layout.dtype != np.float32:
            raise RendererInvalidInputError(
                f"Node layout should be of type np.float32; got {self.node_layout.dtype}"
            )
        if self.node_layout.ndim != 2 or self.node_layout.shape[1] != 2:
            raise RendererInvalidInputError(
                f"Node layout should be a n x 2 array; got {self.node_layout.shape}"
            )
        if self.node_layout.shape[0] == 0:
            raise RendererInvalidInputError(
                f"Node layout must contain at least one node; got {self.node_layout.shape[0]}"
            )
        if self.has_edges:
            if self.edge_data.ndim != 2 or self.edge_data.shape[1] != 2:
                raise RendererInvalidInputError(
                    f"Edge data should be a e x 2 array; got {self.edge_data.shape}"
                )
            if self.weight_data.shape[0] != self.edge_data.shape[0]:
                raise RendererInvalidInputError(
                    f"Weight data should have the same shape as edge_data; "
                    f"weight_data.shape={self.weight_data.shape}, "
                    f"edge_data.shape={self.edge_data.shape}"
                )
        else:
            if self.weight_data.size > 0:
                mglw.logger.warning(
                    "Received empty edge data but non-empty weight data"
                )
        if self.label_data.dtype.type != np.str_:
            raise RendererInvalidInputError(
                f"Label data should be of type np.unicode_; got {self.label_data.dtype}"
            )
        if self.label_data.shape[0] != self.node_layout.shape[0]:
            raise RendererInvalidInputError(
                f"Label data should have same number of nodes as "
                f"node_layout {self.node_layout.shape[0]}; "
                f"got {self.label_data.shape[0]}"
            )

        mglw.logger.info(
            f"Successfully loaded node layout, edge data, weight data, config, and save_path"
        )

        self.node_layout_flattened = self.node_layout.flatten()
        self.config_node_radius: float = self.config["node_radius"]
        self.config_background_color: tuple[int, int, int, int] = tuple(
            self.config["background_color"]
        )
        self.config_edge_segment_width: float = self.config["edge_segment_width"]
        self.config_edge_arrowhead_width: float = self.config["edge_arrowhead_width"]
        self.config_edge_arrowhead_height: float = self.config["edge_arrowhead_height"]
        self.config_arrow_style: int = self.config["arrow_style"]
        self.config_node_fill_color: npt.NDArray[np.float32] = (
            np.array(self.config["node_fill_color"], dtype=np.float32) / 255.0
        )
        self.config_edge_color: npt.NDArray[np.float32] = (
            np.array(self.config["edge_color"], dtype=np.float32) / 255.0
        )
        self.config_has_labels: bool = self.config["has_labels"]
        self.config_label_font_size: float = self.config["label_font_size"]
        self.config_label_font_color: npt.NDArray[np.float32] = (
            np.array(self.config["label_font_color"], dtype=np.float32) / 255.0
        )
        self.config_has_fill = self.config["node_fill_color"][3] > 0 and (
            all(
                f_color != bg_color
                for f_color, bg_color in zip(
                    self.config["node_fill_color"][:3],
                    self.config["background_color"][:3],
                )
            )
        )

        directory = os.path.join(os.path.dirname(__file__), "shaders")
        with (
            open(os.path.join(directory, "node.vert"), "r") as node_vertex_shader,
            open(os.path.join(directory, "node.frag"), "r") as node_fragment_shader,
            open(os.path.join(directory, "node.geom"), "r") as node_geometry_shader,
        ):
            self.node_program = self.ctx.program(
                vertex_shader=node_vertex_shader.read(),
                fragment_shader=node_fragment_shader.read(),
                geometry_shader=node_geometry_shader.read(),
            )
        mglw.logger.info(f"Successfully loaded node shaders")
        mglw.logger.info("Got the following internal members from node shaders:")
        for name in self.node_program:
            member = self.node_program[name]
            mglw.logger.info(f"{name} {type(member)} {member}")
        with (
            open(os.path.join(directory, "edge.vert"), "r") as edge_vertex_shader,
            open(os.path.join(directory, "edge.frag"), "r") as edge_fragment_shader,
            open(os.path.join(directory, "edge.geom"), "r") as edge_geometry_shader,
        ):
            self.edge_program = self.ctx.program(
                vertex_shader=edge_vertex_shader.read(),
                fragment_shader=edge_fragment_shader.read(),
                geometry_shader=edge_geometry_shader.read(),
            )
        mglw.logger.info(f"Successfully loaded edge shaders")
        mglw.logger.info("Got the following internal members from edge shaders:")
        for name in self.edge_program:
            member = self.edge_program[name]
            mglw.logger.info(f"{name} {type(member)} {member}")
        with (
            open(os.path.join(directory, "text.vert"), "r") as text_vertex_shader,
            open(os.path.join(directory, "text.frag"), "r") as text_fragment_shader,
            open(os.path.join(directory, "text.geom"), "r") as text_geometry_shader,
        ):
            self.text_program = self.ctx.program(
                vertex_shader=text_vertex_shader.read(),
                fragment_shader=text_fragment_shader.read(),
                geometry_shader=text_geometry_shader.read(),
            )
        mglw.logger.info(f"Successfully loaded text shaders")
        mglw.logger.info("Got the following internal members from text shaders:")
        for name in self.text_program:
            member = self.text_program[name]
            mglw.logger.info(f"{name} {type(member)} {member}")

        margin = self.config_node_radius * 2 + 50
        fit_ur = np.max(self.node_layout, axis=0)
        fit_dl = np.min(self.node_layout, axis=0)
        center = (fit_ur + fit_dl) / 2
        fit_width = fit_ur[0] - fit_dl[0] + margin
        fit_height = fit_ur[1] - fit_dl[1] + margin
        if fit_width / fit_height > self.aspect_ratio:
            fit_height = fit_width / self.aspect_ratio
        else:
            fit_width = fit_height * self.aspect_ratio

        left = center[0] - fit_width / 2
        bottom = center[1] - fit_height / 2
        right = center[0] + fit_width / 2
        top = center[1] + fit_height / 2
        z_near = -1
        z_far = 1
        self.camera = pyrr.matrix44.create_orthogonal_projection(
            left, right, bottom, top, z_near, z_far, dtype=np.float32
        )

        self.node_mvp = self.node_program["mvp"]
        self.node_mvp.write(self.camera)
        self.node_node_radius = self.node_program["node_radius"]
        self.node_node_radius.value = self.config_node_radius
        self.node_fill_color = self.node_program["fill_color"]
        self.node_fill_color.write(self.config_node_fill_color)
        self.node_vbo = self.ctx.buffer(self.node_layout_flattened)
        self.node_vao = self.ctx.simple_vertex_array(
            self.node_program,
            self.node_vbo,
            "in_vert",
        )

        if self.has_edges:
            self.edge_mvp = self.edge_program["mvp"]
            self.edge_mvp.write(self.camera)
            self.edge_node_radius = self.edge_program["node_radius"]
            self.edge_node_radius.value = self.config_node_radius
            self.edge_edge_segment_width = self.edge_program["edge_segment_width"]
            self.edge_edge_segment_width.value = self.config_edge_segment_width
            self.edge_edge_arrowhead_width = self.edge_program["edge_arrowhead_width"]
            self.edge_edge_arrowhead_width.value = self.config_edge_arrowhead_width
            self.edge_edge_arrowhead_height = self.edge_program["edge_arrowhead_height"]
            self.edge_edge_arrowhead_height.value = self.config_edge_arrowhead_height
            self.edge_edge_color = self.edge_program["edge_color"]
            self.edge_edge_color.value = self.config_edge_color
            self.edge_arrow_style = self.edge_program["arrow_style"]
            self.edge_arrow_style.value = self.config_arrow_style
            self.edge_vbo = self.ctx.buffer(self.node_layout_flattened)
            self.edge_ebo = self.ctx.buffer(self.edge_data)
            self.edge_vao = self.ctx.simple_vertex_array(
                self.edge_program,
                self.edge_vbo,
                "in_vert",
                index_buffer=self.edge_ebo,
                index_element_size=self.edge_data.itemsize,
            )

        if self.config_has_labels:
            mglw.logger.warning("Label support is currently limited.")

            FONT_ASPECT_RATIO = 1238.0 / 2048.0
            CHAR_OFFSET = 32
            TEXTURE_PATH = os.path.join(
                os.path.dirname(__file__), "font", "courier-prime-32-126.png"
            )
            TEXTURE_CHAR_MIN = 32
            TEXTURE_CHAR_MAX = 126

            # check this first to raise errors earlier
            char_count = np.char.str_len(self.label_data)
            max_char_count = np.max(char_count)
            raw_text_centered = np.char.center(self.label_data, max_char_count)

            lookup = np.arange(TEXTURE_CHAR_MAX + 1, dtype=np.uint32)
            raw_text_int = raw_text_centered.view(np.int32)
            if (
                np.min(raw_text_int) < TEXTURE_CHAR_MIN
                or np.max(raw_text_int) > TEXTURE_CHAR_MAX
            ):
                raise ValueError(
                    f"Label contains unsupported characters. "
                    f"Characters should have unicode values "
                    f"between {TEXTURE_CHAR_MIN} and {TEXTURE_CHAR_MAX}"
                )
            text = lookup[raw_text_centered.view(np.int32)]

            total_label_width = (
                max_char_count * self.config_label_font_size * FONT_ASPECT_RATIO
            )
            if max_char_count == 1:
                offset = np.zeros(1, dtype=np.float32)
            else:
                offset = np.linspace(
                    -total_label_width / 2.0, total_label_width / 2.0, max_char_count
                )

            offsets = np.lib.stride_tricks.as_strided(
                offset,
                (self.node_layout.shape[0],) + offset.shape,
                (0,) + offset.strides,
            ).flatten()
            positions = np.repeat(self.node_layout, repeats=max_char_count, axis=0)

            label_buffer_data = (
                np.stack(
                    (positions[:, 0] + offsets, positions[:, 1], text),
                    axis=-1,
                )
                .astype(np.float32)
                .flatten()
            )

            self.text_mvp = self.text_program["mvp"]
            self.text_mvp.write(self.camera)
            self.text_font_size_px = self.text_program["font_size_px"]
            self.text_font_size_px.value = self.config_label_font_size
            self.text_font_aspect_ratio = self.text_program["font_aspect_ratio"]
            self.text_font_aspect_ratio.value = FONT_ASPECT_RATIO
            self.text_char_offset = self.text_program["char_offset"]
            self.text_char_offset.value = CHAR_OFFSET
            self.text_font_color = self.text_program["font_color"]
            self.text_font_color.write(self.config_label_font_color)

            self.text_texture = self.load_texture_array(
                TEXTURE_PATH,
                layers=TEXTURE_CHAR_MAX - TEXTURE_CHAR_MIN + 1,
                flip=False,
            )
            self.text_texture.use()

            self.text_vbo = self.ctx.buffer(label_buffer_data)
            self.text_vao = self.ctx.simple_vertex_array(
                self.text_program,
                self.text_vbo,
                "in_vert",
                "in_char",
            )

    @classmethod
    def add_arguments(cls, parser):
        """Arguments added to grapes's renderer."""
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
            "--weight-data",
            type=str,
            help="Pass the weight data file (.npy) by path.",
        )

        parser.add_argument(
            "--label-data",
            type=str,
            help="Pass the label data file (.npy) by path.",
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

        parser.add_argument(
            "--save-path", type=str, help="Pass where to save a new image."
        )

    def render(self: Self, time, frametime):
        """Renders the graph."""
        self.ctx.clear(
            red=self.config_background_color[0] / 255,
            green=self.config_background_color[1] / 255,
            blue=self.config_background_color[2] / 255,
            alpha=self.config_background_color[3] / 255,
        )
        self.ctx.enable(moderngl.BLEND)

        if self.has_edges:
            self.edge_vao.render(moderngl.LINES)

        self.node_vao.render(moderngl.POINTS)

        if self.config_has_labels:
            self.text_vao.render(moderngl.POINTS)

        if self.save_path is not None:
            image = Image.frombytes(
                "RGBA", self.wnd.fbo.size, self.wnd.fbo.read(components=4)
            ).transpose(Image.FLIP_TOP_BOTTOM)
            image.save(self.save_path)
            self.wnd.close()
