import grapes

import unittest


class DijkstrasCorrectnessTestCase(unittest.TestCase):
    def __init__(self, graph, src, dst, result) -> None:
        super().__init__(f"test")
        self.graph = graph
        self.src = src
        self.dst = dst
        self.result = result

    def test(self):
        self.assertListEqual(
            self.graph.shortest_path(
                self.src, self.dst, algorithm=grapes.ShortestPathAlgorithm.DIJKSTRAS
            ),
            self.result,
        )


class DijkstrasErrorTestCase(unittest.TestCase):
    def test_missing_label(self):
        g = grapes.LabeledGraph()
        try:
            g.shortest_path(1, 2, grapes.ShortestPathAlgorithm.DIJKSTRAS)
        except Exception as exc:
            self.assertIsInstance(exc, grapes.GraphMissingNodeError)

    def test_negative_weight(self):
        g = grapes.LabeledGraph()
        g.add_node(1)
        g.add_node(2)
        try:
            g.shortest_path(1, 2, grapes.ShortestPathAlgorithm.DIJKSTRAS)
        except Exception as exc:
            self.assertIsInstance(exc, grapes.AlgorithmPreconditionError)


def suite():
    suite = unittest.TestSuite()

    g = grapes.LabeledGraph(is_directed=False)
    g.add_node(1)
    g.add_node(2)
    g.add_node(3)
    g.add_node(4)
    g.add_node(5)

    g.add_edge(1, 2, weight=0.5)
    g.add_edge(1, 3, weight=4.0)
    g.add_edge(2, 3, weight=2.5)
    g.add_edge(4, 5, weight=1.0)

    suite.addTest(DijkstrasCorrectnessTestCase(g, 1, 2, [1, 2]))
    suite.addTest(DijkstrasCorrectnessTestCase(g, 1, 3, [1, 2, 3]))
    suite.addTest(DijkstrasCorrectnessTestCase(g, 4, 5, [4, 5]))
    suite.addTest(DijkstrasCorrectnessTestCase(g, 5, 4, [5, 4]))
    suite.addTest(DijkstrasCorrectnessTestCase(g, 1, 4, []))

    suite.addTest(DijkstrasErrorTestCase("test_missing_label"))
    suite.addTest(DijkstrasErrorTestCase("test_negative_weight"))
    return suite


if __name__ == "__main__":
    runner = unittest.TextTestRunner()
    runner.run(suite())
