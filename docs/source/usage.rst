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

* Undirected graph with n nodes

   .. code-block:: python

      import grapes

      n = 10
      g = grapes.Multigraph(False, n)
      print(g.get_node_count())

* Shortest path

   .. code-block:: python

      import grapes

      g = grapes.LabeledGraph()
      g.add_node("A")
      g.add_node("B")
      g.add_node("C")
      g.add_node("D")
      g.add_node("E")
      g.add_node("F")
      g.add_node("G")
      g.add_node("H")

      g.add_edge("A", "B", weight=7)
      g.add_edge("A", "C", weight=9)
      g.add_edge("A", "F", weight=14)
      g.add_edge("B", "C", weight=10)
      g.add_edge("B", "D", weight=15)
      g.add_edge("C", "D", weight=11)
      g.add_edge("C", "F", weight=2)
      g.add_edge("D", "E", weight=6)
      g.add_edge("E", "F", weight=9)
      g.add_edge("G", "H", weight=1)

      print(g.shortest_path("A", "E"))
      print(g.shortest_path("G", "H"))
      print(g.shortest_path("A", "G"))