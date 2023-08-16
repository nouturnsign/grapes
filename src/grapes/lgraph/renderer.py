import numpy.typing as npt

import json
import os

import moderngl
import moderngl_window as mglw
import numpy as np
from PIL import Image


class GraphWindow(mglw.WindowConfig):
    gl_version = (3, 3)
    title = "Grapes Graph"

    def __init__(self, **kwargs):
        super().__init__(**kwargs)
        mglw.logger.info(
            f"Received node_layout={self.argv.node_layout}, edge_data={self.argv.edge_data}, weight_data={self.argv.weight_data}, config={self.argv.config}, and save_path={self.argv.save_path}"
        )
        with (
            open(self.argv.node_layout, "rb") as node_layout,
            open(self.argv.edge_data, "rb") as edge_data,
            open(self.argv.weight_data, "rb") as weight_data,
            open(self.argv.config, "r") as config,
        ):
            self.node_layout: np.ndarray = np.load(node_layout)
            self.edge_data: np.ndarray = np.load(edge_data)
            self.weight_data: np.ndarray = np.load(weight_data)
            self.config: dict = json.load(config)
        if self.argv.delete:
            os.remove(self.argv.node_layout)
            os.remove(self.argv.edge_data)
            os.remove(self.argv.weight_data)
            os.remove(self.argv.config)
        self.save_path = self.argv.save_path
        self.has_edges = self.edge_data.size > 0

        if self.node_layout.dtype != np.float32:
            raise TypeError(
                f"Node layout should be of type np.float32; got {self.node_layout.dtype}"
            )
        if self.node_layout.ndim != 2 or self.node_layout.shape[1] != 2:
            raise TypeError(
                f"Node layout should be a n x 2 array; got {self.node_layout.shape}"
            )
        if self.has_edges:
            if self.edge_data.ndim != 2 or self.edge_data.shape[1] != 2:
                raise TypeError(
                    f"Edge data should be a e x 2 array; got {self.edge_data.shape}"
                )
            if self.weight_data.shape[0] != self.edge_data.shape[0]:
                raise TypeError(
                    f"Weight data should have the same shape as edge_data; weight_data.shape={self.weight_data.shape}, edge_data.shape={self.edge_data.shape}"
                )
        else:
            if self.weight_data.size > 0:
                mglw.logger.warning(
                    "Received empty edge data but non-empty weight data"
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
            "--weight-data",
            type=str,
            help="Pass the weight data file (.npy) by path.",
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

    def render(self, time, frametime):
        self.ctx.clear(
            red=self.config_background_color[0] / 255,
            green=self.config_background_color[1] / 255,
            blue=self.config_background_color[2] / 255,
            alpha=self.config_background_color[3] / 255,
        )
        if self.has_edges:
            self.edge_vao.render(moderngl.LINES)

        self.node_vao.render(moderngl.POINTS)

        if self.save_path is not None:
            image = Image.new("RGBA", self.wnd.fbo.size, self.config_background_color)
            image.paste(
                Image.frombytes(
                    "RGBA", self.wnd.fbo.size, self.wnd.fbo.read(components=4)
                ).transpose(Image.FLIP_TOP_BOTTOM)
            )
            image.save(self.save_path)
            self.wnd.close()
