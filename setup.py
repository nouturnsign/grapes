from setuptools import Extension, setup

grapes_ext = Extension(
    "cgraph",
    sources=["src/grapes/cgraph/cgraph.c", "src/grapes/cgraph/heap.c"],
    include_dirs=["src/grapes/"],
)

setup(ext_package="grapes", ext_modules=[grapes_ext])
