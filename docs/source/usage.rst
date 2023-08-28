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

.. _examples:

Examples
--------

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

* Draw a graph

   .. code-block:: python
      
      import numpy as np

      import grapes

      g = grapes.LabeledGraph(is_directed=True)
      g.add_node("1")
      g.add_node("2")
      g.add_node("3")
      g.add_node("4")

      g.add_edge("1", "3")
      g.add_edge("2", "1")
      g.add_edge("2", "3")
      g.add_edge("3", "4")
      g.add_edge("4", "2")

      g.draw(
         np.array(
            [[0.0, 100.0], [-100.0, 0.0], [100.0, 0.0], [0.0, -100.0]], dtype=np.float32
         ),
         background_color=grapes.colors.BLACK,
         node_radius=20.0,
         node_fill_color=grapes.colors.WHITE,
         edge_segment_width=1.0,
         edge_arrowhead_height=15.0,
         edge_arrowhead_width=5.0,
         edge_color=grapes.colors.WHITE,
         has_labels=True,
         label_font_size=30.0,
         label_font_color=grapes.colors.BLACK,
      )
