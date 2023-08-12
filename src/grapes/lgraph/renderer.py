import json
import os

import moderngl
import moderngl_window as mglw
import numpy as np
from PIL import Image

NODE_LAYOUT_CHUNK_SIZE = 512


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
        node_layout_flattened = self.node_layout.flatten()
        self.node_layout_chunks = np.array_split(
            node_layout_flattened,
            np.arange(
                NODE_LAYOUT_CHUNK_SIZE * 2,
                len(node_layout_flattened),
                NODE_LAYOUT_CHUNK_SIZE * 2,
            ),
        )

        directory = os.path.join(os.path.dirname(__file__), "shaders")
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

        _theta = np.linspace(0, 2 * np.pi, 360)
        self.node_shape = (0.1 * np.dstack((np.cos(_theta), np.sin(_theta)))).astype(
            np.float32
        )

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

        if self.has_edges:
            self.edge_mvp = self.edge_program["mvp"]
            self.edge_vbo = self.ctx.buffer(node_layout_flattened)
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
        self.ctx.clear(1.0, 1.0, 1.0)
        if self.has_edges:
            self.edge_mvp.write(self.camera)
            self.edge_vao.render(moderngl.LINES)
        self.node_mvp.write(self.camera)
        self.node_instance_vbo.write(self.node_shape)
        for node_layout_chunk in self.node_layout_chunks:
            self.node_offsets.write(node_layout_chunk)
            self.node_vao.render(self.node_mode, instances=len(node_layout_chunk) // 2)

        if self.save_path is not None:
            image = Image.frombytes(
                "RGBA", self.wnd.fbo.size, self.wnd.fbo.read(components=4)
            )
            image = image.transpose(Image.FLIP_TOP_BOTTOM)
            image.save(self.save_path)
            self.wnd.close()
