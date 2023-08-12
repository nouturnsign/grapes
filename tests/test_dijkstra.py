import grapes

import unittest


class DijkstrasTestCase(unittest.TestCase):
    def __init__(
        self, graph: grapes.Multigraph, case_number: int, correct_weight: float
    ) -> None:
        super().__init__(f"test{case_number}")
        self.graph = graph
        self.case = case_number
        self.correct_weight = correct_weight

    def test_correctness(self):
        grapes_weight = 0
        self.assertEqual(grapes_weight, self.correct_weight)


def suite():
    suite = unittest.TestSuite()
    suite.addTest(DijkstrasTestCase("test_default_widget_size"))
    return suite


if __name__ == "__main__":
    runner = unittest.TextTestRunner()
    runner.run(suite())
