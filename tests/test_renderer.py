import os

import numpy as np
from PIL import Image

import grapes


def test_renderer_correctness():
    g = grapes.LabeledGraph()
    g.add_node(1)
    assert np.allclose(
        g.compute_circular_layout(1000.0, 0.0, 0.0, 0.0), [[1000.0, 0.0]]
    )
    g.add_node(2)
    assert np.allclose(
        g.compute_circular_layout(1000.0, np.pi / 2, 0.0, 0.0),
        [[0.0, 1000.0], [0.0, -1000.0]],
    )
    g.add_node(3)
    assert np.allclose(
        g.compute_circular_layout(1000.0, 0.0, 500.0, -100.0),
        [[1500.0, -100.0], [0.0, 766.025403784], [0.0, -966.025403784]],
    )
    g.add_node(4)
    layout_old = g.compute_circular_layout(320.0, np.pi / 3.0, 500.0, -100.0)
    assert np.allclose(
        layout_old,
        [
            [660.0, 177.128129211],
            [222.871870789, 60.0],
            [340.0, -377.128129211],
            [777.128129211, -260.0],
        ],
    )
    g.add_node(5)
    g.remove_node(2)
    layout = g.compute_circular_layout(320.0, np.pi / 3.0, 500.0, -100.0)
    assert np.allclose(
        layout,
        layout_old,
    )

    old_limit = Image.MAX_IMAGE_PIXELS
    g.draw(layout, "output.png")
    assert os.path.exists("output.png")
    os.remove("output.png")
    assert Image.MAX_IMAGE_PIXELS == old_limit
