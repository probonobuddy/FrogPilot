#include "frogpilot/ui/qt/offroad/lateral_settings.h"

FrogPilotLateralPanel::FrogPilotLateralPanel(FrogPilotSettingsWindow *parent) : FrogPilotListWidget(parent), parent(parent) {
  QJsonObject shownDescriptions = QJsonDocument::fromJson(QString::fromStdString(params.get("ShownToggleDescriptions")).toUtf8()).object();
  QString className = this->metaObject()->className();

  if (!shownDescriptions.value(className).toBool(false)) {
    forceOpenDescriptions = true;
    shownDescriptions.insert(className, true);
    params.put("ShownToggleDescriptions", QJsonDocument(shownDescriptions).toJson(QJsonDocument::Compact).toStdString());
  }

  QStackedLayout *lateralLayout = new QStackedLayout();
  addItem(lateralLayout);

  FrogPilotListWidget *lateralList = new FrogPilotListWidget(this);

  ScrollView *lateralPanel = new ScrollView(lateralList, this);

  lateralLayout->addWidget(lateralPanel);

  FrogPilotListWidget *advancedLateralTuneList = new FrogPilotListWidget(this);
  FrogPilotListWidget *aolList = new FrogPilotListWidget(this);
  FrogPilotListWidget *laneChangeList = new FrogPilotListWidget(this);
  FrogPilotListWidget *lateralTuneList = new FrogPilotListWidget(this);
  FrogPilotListWidget *qolList = new FrogPilotListWidget(this);

  ScrollView *advancedLateralTunePanel = new ScrollView(advancedLateralTuneList, this);
  ScrollView *aolPanel = new ScrollView(aolList, this);
  ScrollView *laneChangePanel = new ScrollView(laneChangeList, this);
  ScrollView *lateralTunePanel = new ScrollView(lateralTuneList, this);
  ScrollView *qolPanel = new ScrollView(qolList, this);

  lateralLayout->addWidget(advancedLateralTunePanel);
  lateralLayout->addWidget(aolPanel);
  lateralLayout->addWidget(laneChangePanel);
  lateralLayout->addWidget(lateralTunePanel);
  lateralLayout->addWidget(qolPanel);

  const std::vector<std::tuple<QString, QString, QString, QString>> lateralToggles {
    {"AdvancedLateralTune", tr("Advanced Lateral Tuning"), tr("<b>Advanced steering controls for learned steering values, fixed gain overrides, live auto-tuning, and the steering-controller mode openpilot uses.</b>"), "../../frogpilot/assets/toggle_icons/icon_advanced_lateral_tune.png"},
    {"SteerDelay", parent->steerActuatorDelay != 0 ? QString(tr("Actuator Delay (Default: %1)")).arg(QString::number(parent->steerActuatorDelay, 'f', 2)) : tr("Actuator Delay"), tr("<b>Sets how much lag openpilot expects between its steering command and the car responding.</b> The car learns this on its own, so the default is right for almost every car. Raise it if the car reacts late in curves. Lower it if steering feels jumpy."), ""},
    {"SteerFriction", parent->friction != 0 ? QString(tr("Friction (Default: %1)")).arg(QString::number(parent->friction, 'f', 2)) : tr("Friction"), tr("<b>Sets how hard openpilot pushes the wheel to overcome friction near center.</b> The car learns this itself, so most drivers can leave it. Increase it if the wheel sticks near center. Decrease it if it jitters."), ""},
    {"SteerKP", parent->steerKp != 0 ? QString(tr("Kp Factor (Default: %1)")).arg(QString::number(parent->steerKp, 'f', 2)) : tr("Kp Factor"), tr("<b>Controls how hard openpilot works to stay centered.</b> The default uses the car's stock value and is right for almost everyone. Lower it if steering feels persistently twitchy. Raise it if it feels sluggish."), ""},
    {"SteerLatAccel", parent->latAccelFactor != 0 ? QString(tr("Lateral Accel (Default: %1)")).arg(QString::number(parent->latAccelFactor, 'f', 2)) : tr("Lateral Accel"), tr("<b>Sets how strongly openpilot turns the wheel to hold a curve on torque-controlled cars.</b> The car learns this on its own, so leave the default unless a symptom shows up. Increase it if steering feels too sharp or twitchy. Decrease it if it feels weak and slow."), ""},
    {"SteerRatio", parent->steerRatio != 0 ? QString(tr("Steer Ratio (Default: %1)")).arg(QString::number(parent->steerRatio, 'f', 2)) : tr("Steer Ratio"), tr("<b>Sets the steering-wheel-to-road-wheel ratio openpilot steers with.</b> Increase it if steering feels too quick or twitchy. Decrease it if it feels too slow or weak. Auto-learned by default, so you rarely need to touch it."), ""},
    {"ForceAutoTune", tr("Force Auto-Tune On"), tr("<b>Forces openpilot to live-learn its own steering feel, turning off the manual \"Friction\", \"Lateral Accel\", and \"Steer Ratio\" controls while it's on.</b> Turn it on to self-tune those values. Leave it off to hand-tune them. Most drivers are fine with the default."), ""},
    {"ForceAutoTuneOff", tr("Force Auto-Tune Off"), tr("<b>Stops openpilot from auto-learning its own steering feel and locks it to the \"Friction\", \"Lateral Accel\", and \"Steer Ratio\" values you set instead.</b> Turn it on only if you want full manual control of those values. Leave it off to let openpilot keep tuning itself, which suits almost everyone."), ""},
    {"ForceTorqueController", tr("Force Torque Controller"), tr("<b>Switches openpilot to its torque-based steering controller instead of your car's stock one.</b> This can drastically change how the car steers. Only try it if the stock steering feels persistently weak or wandering, and switch it back if it doesn't help.<br><br><i><b>Disclaimer</b>: on a car the torque controller is not tuned for, the wheel may wander, turn too hard, or turn too late.</i>"), ""},

    {"AlwaysOnLateral", tr("Always On Lateral"), tr("<b>openpilot keeps steering even when openpilot is not engaged or you press the gas or brake pedal.</b><br><br><i><b>Disclaimer</b>: pressing the gas or brake does not hand steering back to you, so be ready for the wheel to keep steering itself when you expect to be in control.</i>"), "../../frogpilot/assets/toggle_icons/icon_always_on_lateral.png"},
    {"AlwaysOnLateralMain", tr("Enable With Cruise Control"), tr("<b>Keep \"Always On Lateral\" steering active whenever your car's \"Cruise Control\" is on, even when openpilot is not engaged.</b> Turn this ON for hands-on lane-keeping any time cruise is set. Leave it OFF if you only want steering while openpilot is engaged.<br><br><i><b>Disclaimer</b>: openpilot will steer on its own whenever cruise control is on, even when it appears to be off, so stay alert and keep your hands on the wheel.</i>"), ""},
    {"AlwaysOnLateralLKAS", tr("Enable With LKAS"), tr("<b>Keep \"Always On Lateral\" active whenever Lane Keeping Assist (LKAS) is on, so openpilot steers even when it is not engaged.</b> Turn this ON if you want lane-keeping any time LKAS is on. Leave it OFF to steer only while openpilot is engaged.<br><br><i><b>Disclaimer</b>: the steering wheel can turn on its own whenever LKAS is on, including when openpilot is not engaged, so be ready to take over at any time.</i>"), ""},
    {"PauseAOLOnBrake", tr("Pause on Brake Press Below"), tr("<b>Pause \"Always On Lateral\" while the brake is pressed and you're below the set speed, handing steering back to you.</b> Raise it to take over with a brake tap in more low-speed situations. Set it to \"Off\" to keep \"Always On Lateral\" steering through your braking.<br><br>Default: Off."), ""},

    {"LaneChanges", tr("Lane Changes"), tr("<b>Allow openpilot to change lanes, and manage how it does so: automatic changes, timing, minimum speed, and minimum lane width.</b>"), "../../frogpilot/assets/toggle_icons/icon_lane.png"},
    {"NudgelessLaneChange", tr("Automatic Lane Changes"), tr("<b>When the turn signal is on, openpilot will automatically change lanes.</b> You don't need to nudge the wheel."), ""},
    {"LaneChangeTime", tr("Lane Change Delay"), tr("<b>How long openpilot waits after you turn on the signal before it automatically changes lanes.</b> Raise it to give yourself more time to check and cancel before the car moves. Lower it for quicker, snappier lane changes.<br><br>Default: 1.0 seconds."), ""},
    {"MinimumLaneChangeSpeed", tr("Minimum Lane Change Speed"), tr("<b>Sets the slowest speed at which openpilot will start a lane change.</b> Below this speed, the turn signal won't trigger one. Raise it to keep assisted lane changes to higher-speed roads. Lower it (down to \"Off\") to allow them at lower speeds."), ""},
    {"LaneDetectionWidth", tr("Minimum Lane-Change Width"), tr("<b>Blocks automatic lane changes unless the target lane is at least this wide.</b> Raise it to stop openpilot from auto-merging into narrow lanes, bike lanes, or shoulders. Leave it off to allow any detected lane.<br><br>Default: Off."), ""},
    {"OneLaneChange", tr("One Lane Change Per Signal"), tr("<b>Limit openpilot to one lane change per turn-signal activation.</b> Turn this ON if you only ever want to move over one lane at a time. Leave it OFF to let openpilot chain multiple lane changes while the signal stays on."), ""},

    {"LateralTune", tr("Lateral Tuning"), tr("<b>Steering control changes to fine-tune how openpilot drives.</b>"), "../../frogpilot/assets/toggle_icons/icon_lateral_tune.png"},
    {"TurnDesires", tr("Steer Into Low-Speed Turns"), tr("<b>With the turn signal on below the \"Minimum Lane Change Speed\", openpilot pulls the steering wheel toward the turn instead of holding the lane straight.</b> Leave this off unless you specifically want openpilot to steer low-speed blinker turns at intersections and side streets.<br><br><i><b>Disclaimer</b>: openpilot may turn the wheel toward any side street, driveway, or lane split whenever your blinker is on at low speed, so keep your hands ready to override immediately.</i>"), ""},
    {"NNFF", tr("Neural Network Feedforward (NNFF)"), tr("<b>Steers using a model trained specifically for your car instead of openpilot's standard steering, for smoother, more accurate steering through curves.</b> Turn this on for tighter, more natural steering. Turn it off if you prefer openpilot's stock steering feel."), ""},
    {"NNFFLite", tr("Neural Network Feedforward Lite"), tr("<b>A lighter \"Neural Network Feedforward (NNFF)\" that smooths steering through curves using the model's look-ahead path, without the full neural-network torque model.</b> Turn it on for smoother curve handling when full NNFF isn't available for your car. Turn it off for openpilot's stock steering."), ""},

    {"QOLLateral", tr("Quality of Life"), tr("<b>Low-speed steering convenience settings, including pausing openpilot's steering below a speed you choose.</b>"), "../../frogpilot/assets/toggle_icons/icon_quality_of_life.png"},
    {"PauseLateralSpeed", tr("Pause Steering Below"), tr("<b>Below this speed, openpilot stops steering and hands the wheel back to you until the car speeds back up.</b> Raise it only for low-speed spots where you want to steer yourself. Leave it at or reset it to \"Off\" (default) for normal steering, or use \"Turn Signal Only\" to pause only while signaling.<br><br><i><b>Disclaimer</b>: openpilot will not steer below this speed, so you must steer manually.</i>"), ""}
  };

  for (const auto &[param, title, desc, icon] : lateralToggles) {
    AbstractControl *lateralToggle;

    if (param == "AdvancedLateralTune") {
      FrogPilotManageControl *advancedLateralTuneToggle = new FrogPilotManageControl(param, title, desc, icon);
      QObject::connect(advancedLateralTuneToggle, &FrogPilotManageControl::manageButtonClicked, [lateralLayout, advancedLateralTunePanel]() {
        lateralLayout->setCurrentWidget(advancedLateralTunePanel);
      });
      lateralToggle = advancedLateralTuneToggle;
    } else if (param == "SteerDelay") {
      std::vector<QString> steerDelayButton{"Reset"};
      lateralToggle = new FrogPilotParamValueButtonControl(param, title, desc, icon, 0.01, 1, QString(), std::map<float, QString>(), 0.01, false, {}, steerDelayButton, false, false);
    } else if (param == "SteerFriction") {
      std::vector<QString> steerFrictionButton{"Reset"};
      lateralToggle = new FrogPilotParamValueButtonControl(param, title, desc, icon, 0, 0.5, QString(), std::map<float, QString>(), 0.01, false, {}, steerFrictionButton, false, false);
    } else if (param == "SteerKP") {
      std::vector<QString> steerKPButton{"Reset"};
      lateralToggle = new FrogPilotParamValueButtonControl(param, title, desc, icon, parent->steerKp * 0.5, parent->steerKp * 1.5, QString(), std::map<float, QString>(), 0.01, false, {}, steerKPButton, false, false);
    } else if (param == "SteerLatAccel") {
      std::vector<QString> steerLatAccelButton{"Reset"};
      lateralToggle = new FrogPilotParamValueButtonControl(param, title, desc, icon, parent->latAccelFactor * 0.75, parent->latAccelFactor * 1.25, QString(), std::map<float, QString>(), 0.01, false, {}, steerLatAccelButton, false, false);
    } else if (param == "SteerRatio") {
      std::vector<QString> steerRatioButton{"Reset"};
      lateralToggle = new FrogPilotParamValueButtonControl(param, title, desc, icon, parent->steerRatio * 0.5, parent->steerRatio * 1.5, QString(), std::map<float, QString>(), 0.01, false, {}, steerRatioButton, false, false);

    } else if (param == "AlwaysOnLateral") {
      FrogPilotManageControl *aolToggle = new FrogPilotManageControl(param, title, desc, icon);
      QObject::connect(aolToggle, &FrogPilotManageControl::manageButtonClicked, [lateralLayout, aolPanel]() {
        lateralLayout->setCurrentWidget(aolPanel);
      });
      lateralToggle = aolToggle;
    } else if (param == "PauseAOLOnBrake") {
      lateralToggle = new FrogPilotParamValueControl(param, title, desc, icon, 0, 99, QString(), std::map<float, QString>(), 1, true);

    } else if (param == "LaneChanges") {
      FrogPilotManageControl *laneChangeToggle = new FrogPilotManageControl(param, title, desc, icon);
      QObject::connect(laneChangeToggle, &FrogPilotManageControl::manageButtonClicked, [lateralLayout, laneChangePanel]() {
        lateralLayout->setCurrentWidget(laneChangePanel);
      });
      lateralToggle = laneChangeToggle;
    } else if (param == "LaneChangeTime") {
      std::map<float, QString> laneChangeTimeLabels;
      for (float i = 0; i <= 5; i += 0.1) {
        laneChangeTimeLabels[i] = i == 0 ? tr("Instant") : std::lround(i / 0.1) == 1 / 0.1 ? QString::number(i, 'f', 1) + tr(" second") : QString::number(i, 'f', 1) + tr(" seconds");
      }
      lateralToggle = new FrogPilotParamValueControl(param, title, desc, icon, 0, 5, QString(), laneChangeTimeLabels, 0.1);
    } else if (param == "LaneDetectionWidth") {
      lateralToggle = new FrogPilotParamValueControl(param, title, desc, icon, 0, 15, QString(), std::map<float, QString>(), 0.1, true);
    } else if (param == "MinimumLaneChangeSpeed") {
      lateralToggle = new FrogPilotParamValueControl(param, title, desc, icon, 0, 99, QString(), std::map<float, QString>(), 1, true);

    } else if (param == "LateralTune") {
      FrogPilotManageControl *lateralTuneToggle = new FrogPilotManageControl(param, title, desc, icon);
      QObject::connect(lateralTuneToggle, &FrogPilotManageControl::manageButtonClicked, [lateralLayout, lateralTunePanel]() {
        lateralLayout->setCurrentWidget(lateralTunePanel);
      });
      lateralToggle = lateralTuneToggle;

    } else if (param == "QOLLateral") {
      FrogPilotManageControl *qolLateralToggle = new FrogPilotManageControl(param, title, desc, icon);
      QObject::connect(qolLateralToggle, &FrogPilotManageControl::manageButtonClicked, [lateralLayout, qolPanel]() {
        lateralLayout->setCurrentWidget(qolPanel);
      });
      lateralToggle = qolLateralToggle;
    } else if (param == "PauseLateralSpeed") {
      std::vector<QString> pauseLateralToggles{"PauseLateralOnSignal"};
      std::vector<QString> pauseLateralToggleNames{tr("Turn Signal Only")};
      lateralToggle = new FrogPilotParamValueButtonControl(param, title, desc, icon, 0, 99, QString(), std::map<float, QString>(), 1, true, pauseLateralToggles, pauseLateralToggleNames, true);

    } else {
      lateralToggle = new ParamControl(param, title, desc, icon);
    }

    toggles[param] = lateralToggle;

    if (advancedLateralTuneKeys.contains(param)) {
      advancedLateralTuneList->addItem(lateralToggle);
    } else if (aolKeys.contains(param)) {
      aolList->addItem(lateralToggle);
    } else if (laneChangeKeys.contains(param)) {
      laneChangeList->addItem(lateralToggle);
    } else if (lateralTuneKeys.contains(param)) {
      lateralTuneList->addItem(lateralToggle);
    } else if (qolKeys.contains(param)) {
      qolList ->addItem(lateralToggle);
    } else {
      lateralList->addItem(lateralToggle);

      parentKeys.insert(param);
    }

    if (FrogPilotManageControl *frogPilotManageToggle = qobject_cast<FrogPilotManageControl*>(lateralToggle)) {
      QObject::connect(frogPilotManageToggle, &FrogPilotManageControl::manageButtonClicked, [this]() {
        emit openSubPanel();
        openDescriptions(forceOpenDescriptions, toggles);
      });
    }

    QObject::connect(lateralToggle, &AbstractControl::hideDescriptionEvent, [this]() {
      update();
    });
    QObject::connect(lateralToggle, &AbstractControl::showDescriptionEvent, [this]() {
      update();
    });
  }

  QSet<QString> forceUpdateKeys = {"ForceAutoTune", "ForceAutoTuneOff", "LateralTune", "NNFF", "NudgelessLaneChange"};
  for (const QString &key : forceUpdateKeys) {
    QObject::connect(static_cast<ToggleControl*>(toggles[key]), &ToggleControl::toggleFlipped, this, &FrogPilotLateralPanel::updateToggles);
  }

  QSet<QString> rebootKeys = {"AlwaysOnLateral", "ForceTorqueController", "NNFF", "NNFFLite"};
  for (const QString &key : rebootKeys) {
    QObject::connect(static_cast<ToggleControl*>(toggles[key]), &ToggleControl::toggleFlipped, [key, this](bool state) {
      if (started) {
        if (key == "AlwaysOnLateral" && state) {
          if (FrogPilotConfirmationDialog::toggleReboot(this)) {
            Hardware::reboot();
          }
        } else if (key != "AlwaysOnLateral") {
          if (FrogPilotConfirmationDialog::toggleReboot(this)) {
            Hardware::reboot();
          }
        }
      }
    });
  }

  steerDelayToggle = static_cast<FrogPilotParamValueButtonControl*>(toggles["SteerDelay"]);
  QObject::connect(steerDelayToggle, &FrogPilotParamValueButtonControl::buttonClicked, [parent, this]() {
    if (FrogPilotConfirmationDialog::yesorno(tr("Reset <b>Actuator Delay</b> to its default value?"), this)) {
      params.putFloat("SteerDelay", parent->steerActuatorDelay);
      steerDelayToggle->refresh();
    }
  });

  steerFrictionToggle = static_cast<FrogPilotParamValueButtonControl*>(toggles["SteerFriction"]);
  QObject::connect(steerFrictionToggle, &FrogPilotParamValueButtonControl::buttonClicked, [parent, this]() {
    if (FrogPilotConfirmationDialog::yesorno(tr("Reset <b>Friction</b> to its default value?"), this)) {
      params.putFloat("SteerFriction", parent->friction);
      steerFrictionToggle->refresh();
    }
  });

  steerKPToggle = static_cast<FrogPilotParamValueButtonControl*>(toggles["SteerKP"]);
  QObject::connect(steerKPToggle, &FrogPilotParamValueButtonControl::buttonClicked, [parent, this]() {
    if (FrogPilotConfirmationDialog::yesorno(tr("Reset <b>Kp Factor</b> to its default value?"), this)) {
      params.putFloat("SteerKP", parent->steerKp);
      steerKPToggle->refresh();
    }
  });

  steerLatAccelToggle = static_cast<FrogPilotParamValueButtonControl*>(toggles["SteerLatAccel"]);
  QObject::connect(steerLatAccelToggle, &FrogPilotParamValueButtonControl::buttonClicked, [parent, this]() {
    if (FrogPilotConfirmationDialog::yesorno(tr("Reset <b>Lateral Accel</b> to its default value?"), this)) {
      params.putFloat("SteerLatAccel", parent->latAccelFactor);
      steerLatAccelToggle->refresh();
    }
  });

  steerRatioToggle = static_cast<FrogPilotParamValueButtonControl*>(toggles["SteerRatio"]);
  QObject::connect(steerRatioToggle, &FrogPilotParamValueButtonControl::buttonClicked, [parent, this]() {
    if (FrogPilotConfirmationDialog::yesorno(tr("Reset <b>Steer Ratio</b> to its default value?"), this)) {
      params.putFloat("SteerRatio", parent->steerRatio);
      steerRatioToggle->refresh();
    }
  });

  openDescriptions(forceOpenDescriptions, toggles);

  QObject::connect(parent, &FrogPilotSettingsWindow::closeSubPanel, [lateralLayout, lateralPanel, this] {
    openDescriptions(forceOpenDescriptions, toggles);
    lateralLayout->setCurrentWidget(lateralPanel);
  });
  QObject::connect(parent, &FrogPilotSettingsWindow::updateMetric, this, &FrogPilotLateralPanel::updateMetric);
  QObject::connect(uiState(), &UIState::uiUpdate, this, &FrogPilotLateralPanel::updateState);
}

void FrogPilotLateralPanel::showEvent(QShowEvent *event) {
  frogpilotToggleLevels = parent->frogpilotToggleLevels;

  steerDelayToggle->setTitle(QString(tr("Actuator Delay (Default: %1)")).arg(QString::number(parent->steerActuatorDelay, 'f', 2)));
  steerFrictionToggle->setTitle(QString(tr("Friction (Default: %1)")).arg(QString::number(parent->friction, 'f', 2)));
  steerKPToggle->setTitle(QString(tr("Kp Factor (Default: %1)")).arg(QString::number(parent->steerKp, 'f', 2)));
  steerKPToggle->updateControl(parent->steerKp * 0.5, parent->steerKp * 1.5);
  steerLatAccelToggle->setTitle(QString(tr("Lateral Accel (Default: %1)")).arg(QString::number(parent->latAccelFactor, 'f', 2)));
  steerLatAccelToggle->updateControl(parent->latAccelFactor * 0.75, parent->latAccelFactor * 1.25);
  steerRatioToggle->setTitle(QString(tr("Steer Ratio (Default: %1)")).arg(QString::number(parent->steerRatio, 'f', 2)));
  steerRatioToggle->updateControl(parent->steerRatio * 0.5, parent->steerRatio * 1.5);

  updateToggles();
}

void FrogPilotLateralPanel::updateState(const UIState &s) {
  if (!isVisible()) return;

  started = s.scene.started;
}

void FrogPilotLateralPanel::updateMetric(bool metric, bool bootRun) {
  static bool previousMetric;
  if (metric != previousMetric && !bootRun) {
    double distanceConversion = metric ? FOOT_TO_METER : METER_TO_FOOT;
    double speedConversion = metric ? MILE_TO_KM : KM_TO_MILE;

    params.putFloatNonBlocking("LaneDetectionWidth", params.getFloat("LaneDetectionWidth") * distanceConversion);

    params.putIntNonBlocking("MinimumLaneChangeSpeed", params.getInt("MinimumLaneChangeSpeed") * speedConversion);
    params.putIntNonBlocking("PauseAOLOnBrake", params.getInt("PauseAOLOnBrake") * speedConversion);
    params.putIntNonBlocking("PauseLateralSpeed", params.getInt("PauseLateralSpeed") * speedConversion);
  }
  previousMetric = metric;

  static std::map<float, QString> imperialDistanceLabels;
  static std::map<float, QString> imperialSpeedLabels;
  static std::map<float, QString> metricDistanceLabels;
  static std::map<float, QString> metricSpeedLabels;

  static bool labelsInitialized = false;
  if (!labelsInitialized) {
    for (int i = 0; i <= 150; ++i) {
      float key = i / 10.0f;
      imperialDistanceLabels[key] = key == 0 ? tr("Off") : i == 1 ? QString::number(i) + tr(" foot") : QString::number(key, 'f', 1) + tr(" feet");
    }

    for (int i = 0; i <= 99; ++i) {
      imperialSpeedLabels[i] = i == 0 ? tr("Off") : QString::number(i) + tr(" mph");
    }

    for (int i = 0; i <= 50; ++i) {
      float key = i / 10.0f;
      metricDistanceLabels[key] = key == 0 ? tr("Off") : i == 1 ? QString::number(i) + tr(" meter") : QString::number(key, 'f', 1) + tr(" meters");
    }

    for (int i = 0; i <= 150; ++i) {
      metricSpeedLabels[i] = i == 0 ? tr("Off") : QString::number(i) + tr(" km/h");
    }

    labelsInitialized = true;
  }

  FrogPilotParamValueControl *laneWidthToggle = static_cast<FrogPilotParamValueControl*>(toggles["LaneDetectionWidth"]);
  FrogPilotParamValueControl *minimumLaneChangeSpeedToggle = static_cast<FrogPilotParamValueControl*>(toggles["MinimumLaneChangeSpeed"]);
  FrogPilotParamValueControl *pauseAOLOnBrakeToggle = static_cast<FrogPilotParamValueControl*>(toggles["PauseAOLOnBrake"]);
  FrogPilotParamValueControl *pauseLateralToggle = static_cast<FrogPilotParamValueControl*>(toggles["PauseLateralSpeed"]);

  if (metric) {
    laneWidthToggle->updateControl(0, 5, metricDistanceLabels);

    minimumLaneChangeSpeedToggle->updateControl(0, 150, metricSpeedLabels);
    pauseAOLOnBrakeToggle->updateControl(0, 150, metricSpeedLabels);
    pauseLateralToggle->updateControl(0, 150, metricSpeedLabels);
  } else {
    laneWidthToggle->updateControl(0, 15, imperialDistanceLabels);

    minimumLaneChangeSpeedToggle->updateControl(0, 99, imperialSpeedLabels);
    pauseAOLOnBrakeToggle->updateControl(0, 99, imperialSpeedLabels);
    pauseLateralToggle->updateControl(0, 99, imperialSpeedLabels);
  }
}

void FrogPilotLateralPanel::updateToggles() {
  for (auto &[key, toggle] : toggles) {
    if (parentKeys.contains(key)) {
      toggle->setVisible(false);
    }
  }

  bool forcingAutoTune = !parent->hasAutoTune && params.getBool("ForceAutoTune");
  bool forcingAutoTuneOff = parent->hasAutoTune && params.getBool("ForceAutoTuneOff");
  bool forcingTorqueController = !parent->isAngleCar && params.getBool("ForceTorqueController");
  bool usingNNFF = parent->hasNNFFLog && params.getBool("LateralTune") && params.getBool("NNFF");

  for (auto &[key, toggle] : toggles) {
    if (parentKeys.contains(key)) {
      continue;
    }

    bool setVisible = parent->tuningLevel >= frogpilotToggleLevels[key].toDouble();

    if (key == "AlwaysOnLateralLKAS") {
      setVisible &= parent->isHKGCanFd;
      setVisible &= !parent->hasOpenpilotLongitudinal;
    }

    else if (key == "AlwaysOnLateralMain") {
      setVisible &= !parent->isHKGCanFd;
      setVisible |= parent->hasOpenpilotLongitudinal;
    }

    else if (key == "ForceAutoTune") {
      setVisible &= !parent->hasAutoTune;
      setVisible &= !parent->isAngleCar;
      setVisible &= parent->isTorqueCar || forcingTorqueController || usingNNFF;
    }

    else if (key == "ForceAutoTuneOff") {
      setVisible &= parent->hasAutoTune;
    }

    else if (key == "ForceTorqueController") {
      setVisible &= !parent->isAngleCar;
      setVisible &= !parent->isTorqueCar;
    }

    else if (key == "LaneChangeTime") {
      setVisible &= params.getBool("LaneChanges") && params.getBool("NudgelessLaneChange");
    }

    else if (key == "LaneDetectionWidth") {
      setVisible &= params.getBool("LaneChanges") && params.getBool("NudgelessLaneChange");
    }

    else if (key == "NNFF") {
      setVisible &= parent->hasNNFFLog;
      setVisible &= !parent->isAngleCar;
    }

    else if (key == "NNFFLite") {
      setVisible &= !usingNNFF;
      setVisible &= !parent->isAngleCar;
    }

    else if (key == "SteerDelay") {
      setVisible &= parent->steerActuatorDelay != 0;
    }

    else if (key == "SteerFriction") {
      setVisible &= parent->friction != 0;
      setVisible &= parent->hasAutoTune ? forcingAutoTuneOff : !forcingAutoTune;
      setVisible &= parent->isTorqueCar || forcingTorqueController || usingNNFF;
      setVisible &= !usingNNFF;
    }

    else if (key == "SteerKP") {
      setVisible &= parent->steerKp != 0;
      setVisible &= parent->isTorqueCar || forcingTorqueController || usingNNFF;
      setVisible &= !parent->isAngleCar;
    }

    else if (key == "SteerLatAccel") {
      setVisible &= parent->latAccelFactor != 0;
      setVisible &= parent->hasAutoTune ? forcingAutoTuneOff : !forcingAutoTune;
      setVisible &= parent->isTorqueCar || forcingTorqueController || usingNNFF;
      setVisible &= !usingNNFF;
    }

    else if (key == "SteerRatio") {
      setVisible &= parent->steerRatio != 0;
      setVisible &= parent->hasAutoTune ? forcingAutoTuneOff : !forcingAutoTune;
    }

    toggle->setVisible(setVisible);

    if (setVisible) {
      if (advancedLateralTuneKeys.contains(key)) {
        toggles["AdvancedLateralTune"]->setVisible(true);
      } else if (aolKeys.contains(key)) {
        toggles["AlwaysOnLateral"]->setVisible(true);
      } else if (laneChangeKeys.contains(key)) {
        toggles["LaneChanges"]->setVisible(true);
      } else if (lateralTuneKeys.contains(key)) {
        toggles["LateralTune"]->setVisible(true);
      } else if (qolKeys.contains(key)) {
        toggles["QOLLateral"]->setVisible(true);
      }
    }
  }

  openDescriptions(forceOpenDescriptions, toggles);

  update();
}
