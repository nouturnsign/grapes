import numpy as np

from setuptools import Extension, setup

grapes_ext = Extension(
    "cgraph",
    sources=[
        "src/grapes/cgraph/cgraph.c",
        "src/grapes/cgraph/deque.c",
        "src/grapes/cgraph/heap.c",
        "src/grapes/cgraph/trav.c",
    ],
    include_dirs=["src/grapes/cgraph", np.get_include()],
)

setup(ext_package="grapes", ext_modules=[grapes_ext])
