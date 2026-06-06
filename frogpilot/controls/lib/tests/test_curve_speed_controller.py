#!/usr/bin/env python3
import importlib.util
import sys
import unittest

from pathlib import Path
from types import ModuleType, SimpleNamespace


class FakeParams:
  def __init__(self):
    self.values = {}

  def put_float_nonblocking(self, key, value):
    self.values[key] = value


def load_curve_speed_controller():
  fake_params = FakeParams()

  realtime = ModuleType("openpilot.common.realtime")
  realtime.DT_MDL = 0.05

  variables = ModuleType("openpilot.frogpilot.common.frogpilot_variables")
  variables.CRUISING_SPEED = 5
  variables.DEFAULT_LATERAL_ACCELERATION = 2.0
  variables.PLANNER_TIME = 10
  variables.params = fake_params

  sys.modules["openpilot.common.realtime"] = realtime
  sys.modules["openpilot.frogpilot.common.frogpilot_variables"] = variables

  module_path = Path(__file__).parents[1] / "curve_speed_controller.py"
  spec = importlib.util.spec_from_file_location("curve_speed_controller_under_test", module_path)
  module = importlib.util.module_from_spec(spec)
  spec.loader.exec_module(module)
  return module, fake_params


class TestCurveSpeedController(unittest.TestCase):
  def setUp(self):
    self.module, self.params = load_curve_speed_controller()
    self.controller = self.module.CurveSpeedController.__new__(self.module.CurveSpeedController)

  def test_caps_existing_calibration_data(self):
    self.controller.curvature_data = {
      "0.01": {"average": 2.4, "count": 10},
      "0.02": {"average": 2.8, "count": 10},
    }

    self.controller.update_lateral_acceleration()

    self.assertEqual(self.controller.lateral_acceleration, 2.0)
    self.assertEqual(self.params.values["CalibratedLateralAcceleration"], 2.0)

  def test_caps_new_training_sample(self):
    self.controller.curvature_data = {}
    self.controller.enable_training = False
    self.controller.training_timer = self.module.PLANNER_TIME
    self.controller.frogpilot_planner = SimpleNamespace(
      driving_in_curve=True,
      lateral_acceleration=2.7,
      road_curvature=0.01,
      tracking_lead=False,
    )
    sm = {
      "carControl": SimpleNamespace(longActive=False),
      "carState": SimpleNamespace(leftBlinker=False, rightBlinker=False),
    }

    self.controller.log_data(self.module.CRUISING_SPEED + 1, sm)

    self.assertEqual(self.controller.curvature_data["0.01"]["average"], 2.0)
    self.assertEqual(self.controller.lateral_acceleration, 2.0)


if __name__ == "__main__":
  unittest.main()
