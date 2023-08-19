import os

from PIL import Image

import grapes


def test_renderer_correctness():
    g = grapes.LabeledGraph()
    g.add_node(1)

    old_limit = Image.MAX_IMAGE_PIXELS
    g.draw(g.compute_circular_layout(), "output.png")
    assert os.path.exists("output.png")
    os.remove("output.png")
    assert Image.MAX_IMAGE_PIXELS == old_limit
