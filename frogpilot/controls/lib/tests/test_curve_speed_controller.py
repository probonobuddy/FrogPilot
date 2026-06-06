#!/usr/bin/env python3
import importlib.util
import sys
import unittest

from pathlib import Path
from types import ModuleType, SimpleNamespace


def load_curve_speed_controller():
  variables = ModuleType("openpilot.frogpilot.common.frogpilot_variables")
  variables.CRUISING_SPEED = 5
  variables.DEFAULT_LATERAL_ACCELERATION = 2.0

  sys.modules["openpilot.frogpilot.common.frogpilot_variables"] = variables

  module_path = Path(__file__).parents[1] / "curve_speed_controller.py"
  spec = importlib.util.spec_from_file_location("curve_speed_controller_under_test", module_path)
  module = importlib.util.module_from_spec(spec)
  spec.loader.exec_module(module)
  return module


class TestCurveSpeedController(unittest.TestCase):
  def setUp(self):
    self.module = load_curve_speed_controller()
    weather = SimpleNamespace(weather_id=0, reduce_lateral_acceleration=0.0)
    planner = SimpleNamespace(road_curvature=0.01, frogpilot_weather=weather)
    vcruise = SimpleNamespace(frogpilot_planner=planner)
    self.controller = self.module.CurveSpeedController(vcruise)

  def test_historical_default_formula(self):
    toggles = SimpleNamespace(curve_sensitivity=1.0, turn_aggressiveness=1.0)

    self.controller.update_target(toggles)

    self.assertAlmostEqual(self.controller.target, (2.0 / 0.01)**0.5)

  def test_manual_settings_adjust_target(self):
    conservative = SimpleNamespace(curve_sensitivity=1.5, turn_aggressiveness=0.5)
    aggressive = SimpleNamespace(curve_sensitivity=0.5, turn_aggressiveness=1.5)

    self.controller.update_target(conservative)
    conservative_target = self.controller.target
    self.controller.update_target(aggressive)

    self.assertLess(conservative_target, self.controller.target)

  def test_weather_reduction_is_preserved(self):
    self.controller.frogpilot_planner.frogpilot_weather.weather_id = 1
    self.controller.frogpilot_planner.frogpilot_weather.reduce_lateral_acceleration = 0.25
    toggles = SimpleNamespace(curve_sensitivity=1.0, turn_aggressiveness=1.0)

    self.controller.update_target(toggles)

    self.assertAlmostEqual(self.controller.target, (1.5 / 0.01)**0.5)

  def test_target_does_not_drop_below_cruising_speed(self):
    self.controller.frogpilot_planner.road_curvature = 1.0
    toggles = SimpleNamespace(curve_sensitivity=1.5, turn_aggressiveness=0.5)

    self.controller.update_target(toggles)

    self.assertEqual(self.controller.target, self.module.CRUISING_SPEED)


if __name__ == "__main__":
  unittest.main()
