# Configuration file for the Sphinx documentation builder.
#
# For the full list of built-in configuration values, see the documentation:
# https://www.sphinx-doc.org/en/master/usage/configuration.html

import os
import sys

sys.path.insert(0, os.path.abspath("../../src"))

# -- Project information -----------------------------------------------------
# https://www.sphinx-doc.org/en/master/usage/configuration.html#project-information

project = "grapes-graph"
copyright = "2023, Eric Wang"
author = "Eric Wang"
release = "0.1.2"

# -- General configuration ---------------------------------------------------
# https://www.sphinx-doc.org/en/master/usage/configuration.html#general-configuration

extensions = [
    "sphinx.ext.duration",
    "sphinx.ext.autodoc",
    "autoapi.extension",
]
autoapi_dirs = [os.path.abspath("../../src/grapes")]
autoapi_add_toctree_entry = False
autoapi_python_use_implicit_namespaces = True

templates_path = ["_templates"]
exclude_patterns = []


def skip_cgraph_and_lgraph(app, what, name, obj, skip, options):
    if "cgraph" in name or "lgraph" in name:
        skip = True
    return skip


def setup(sphinx):
    sphinx.connect("autoapi-skip-member", skip_cgraph_and_lgraph)


# -- Options for HTML output -------------------------------------------------
# https://www.sphinx-doc.org/en/master/usage/configuration.html#options-for-html-output

html_theme = "furo"
html_static_path = ["_static"]
