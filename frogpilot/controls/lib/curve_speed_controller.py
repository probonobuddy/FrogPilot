#!/usr/bin/env python3
import numpy as np

from openpilot.frogpilot.common.frogpilot_variables import CRUISING_SPEED, DEFAULT_LATERAL_ACCELERATION

MIN_CURVATURE = 1e-6


class CurveSpeedController:
  def __init__(self, FrogPilotVCruise):
    self.frogpilot_planner = FrogPilotVCruise.frogpilot_planner

    # The deterministic controller never trains, but frogpilotPlan still
    # publishes this compatibility field.
    self.enable_training = False
    self.target = 0.0

  def update_target(self, frogpilot_toggles):
    lateral_acceleration = DEFAULT_LATERAL_ACCELERATION * frogpilot_toggles.turn_aggressiveness
    if self.frogpilot_planner.frogpilot_weather.weather_id != 0:
      lateral_acceleration *= 1 - self.frogpilot_planner.frogpilot_weather.reduce_lateral_acceleration

    adjusted_curvature = max(abs(self.frogpilot_planner.road_curvature) * frogpilot_toggles.curve_sensitivity, MIN_CURVATURE)
    self.target = max(float(np.sqrt(lateral_acceleration / adjusted_curvature)), CRUISING_SPEED)
