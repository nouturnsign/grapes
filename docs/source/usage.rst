Usage
=====

.. _installation:

Installation
------------

To use grapes, first install it using pip:

.. code-block:: console

   (.venv) $ pip install grapes-graph

Then, it can be imported.

.. code-block:: python

   import grapes

Examples
--------

* Empty graph with n nodes

   .. code-block:: python

      import grapes

      n = 10
      g = grapes.Graph(n)
      print(g.get_node_count())
