from setuptools import Extension, setup

grapes_ext = Extension(
    "cgraph",
    sources=["grapes/cgraph/cgraph.c", "grapes/cgraph/heap.c"],
    include_dirs=["grapes/"],
)

setup(ext_package="grapes", ext_modules=[grapes_ext])
