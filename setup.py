from setuptools import Extension, setup

grapes_ext = Extension(
    "grapes",
    sources=["grapes/grapesmodule.c", "grapes/heap.c"],
    include_dirs=["grapes/"],
)

setup(ext_modules=[grapes_ext])
