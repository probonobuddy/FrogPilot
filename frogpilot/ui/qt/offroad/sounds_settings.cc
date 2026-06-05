#include "frogpilot/ui/qt/offroad/sounds_settings.h"

void playSound(const QString &alert, int volume) {
  QString stockPath = "../../selfdrive/assets/sounds/" + alert + ".wav";
  QString themePath = "../../frogpilot/assets/active_theme/sounds/" + alert + ".wav";

  QString filePath = QFile::exists(themePath) ? themePath : stockPath;

  QProcess::execute("pkill", {"-f", "ffplay"});

  int clampedVolume = std::clamp(volume, 0, 100);

  QProcess::startDetached("ffplay", {"-nodisp", "-autoexit", "-volume", QString::number(clampedVolume), filePath});
}

FrogPilotSoundsPanel::FrogPilotSoundsPanel(FrogPilotSettingsWindow *parent) : FrogPilotListWidget(parent), parent(parent) {
  QJsonObject shownDescriptions = QJsonDocument::fromJson(QString::fromStdString(params.get("ShownToggleDescriptions")).toUtf8()).object();
  QString className = this->metaObject()->className();

  if (!shownDescriptions.value(className).toBool(false)) {
    forceOpenDescriptions = true;
    shownDescriptions.insert(className, true);
    params.put("ShownToggleDescriptions", QJsonDocument(shownDescriptions).toJson(QJsonDocument::Compact).toStdString());
  }

  QStackedLayout *soundsLayout = new QStackedLayout();
  addItem(soundsLayout);

  FrogPilotListWidget *soundsList = new FrogPilotListWidget(this);

  ScrollView *soundsPanel = new ScrollView(soundsList, this);

  soundsLayout->addWidget(soundsPanel);

  FrogPilotListWidget *alertVolumeControlList = new FrogPilotListWidget(this);
  FrogPilotListWidget *customAlertsList = new FrogPilotListWidget(this);

  ScrollView *alertVolumeControlPanel = new ScrollView(alertVolumeControlList, this);
  ScrollView *customAlertsPanel = new ScrollView(customAlertsList, this);

  soundsLayout->addWidget(alertVolumeControlPanel);
  soundsLayout->addWidget(customAlertsPanel);

  const std::vector<std::tuple<QString, QString, QString, QString>> soundsToggles {
    {"AlertVolumeControl", tr("Alert Volume Controller"), tr("<b>Control how loud each type of openpilot alert sounds.</b>"), "../../frogpilot/assets/toggle_icons/icon_mute.png"},
    {"DisengageVolume", tr("Disengage Volume"), tr("<b>Sets how loud the chime is when openpilot disengages and hands control back to you.</b> At \"Muted\" you may not notice the handoff. Default is Auto, which matches the chime to cabin noise. Turn it up so the handoff is hard to miss, and only turn it down while the chime stays clearly audible.<br><br><i><b>Disclaimer</b>: muting silences the cue that openpilot has stopped steering, braking, and controlling speed.</i>"), ""},
    {"EngageVolume", tr("Engage Volume"), tr("<b>Sets how loud the chime is when openpilot engages</b>, such as after pressing the \"RESUME\" or \"SET\" steering wheel buttons. Raise it if you keep missing when openpilot turns on. Lower it (or set \"Muted\") if the chime is annoying.<br><br>Default: Auto (matches the volume to cabin noise)."), ""},
    {"PromptVolume", tr("Prompt Volume"), tr("<b>Muting this silences openpilot's attention prompts, including its blind-spot, lane-departure, and steering-limit warnings.</b> The default is Auto. Push it higher to hear prompts over road noise, and only ease it back while they stay easy to catch.<br><br><i><b>Disclaimer</b>: at \"Muted\" you may get no audible warning when a car is in your blind spot or openpilot reaches its steering limit.</i>"), ""},
    {"PromptDistractedVolume", tr("Prompt Distracted Volume"), tr("<b>Sets how loud openpilot's early \"Pay Attention\" and \"Touch Steering Wheel\" nudges are when driver monitoring first sees you distracted or unresponsive.</b> Auto is the default. Turn it up for a noisier cabin, and turn it down only while the cue stays easy to hear.<br><br><i><b>Disclaimer</b>: muting silences this early nudge, so you may not hear openpilot ask for your attention until the louder take-over warning sounds.</i>"), ""},
    {"RefuseVolume", tr("Refuse Volume"), tr("<b>Set the volume for the chime openpilot plays when it refuses to engage</b>, such as \"Brake Hold Active\", \"Door Open\", or \"Seatbelt Unlatched\". Raise it if you miss that openpilot refused to turn on. Lower it or set \"Muted\" if the chime is annoying.<br><br>Default: Auto."), ""},
    {"WarningSoftVolume", tr("Warning Soft Volume"), tr("<b>Lowering this makes openpilot's softer risk warnings easier to miss.</b> These include \"BRAKE! Risk of Collision\" and \"Steering Temporarily Unavailable\". The default is Auto. Set it higher if these alerts are hard to hear, and lower only as far as they stay easy to notice.<br><br><i><b>Disclaimer</b>: the slider can't go below 25%, so these warnings still play even at the lowest setting.</i>"), ""},
    {"WarningImmediateVolume", tr("Take-Over Alert Volume"), tr("<b>Lowering this makes openpilot's loudest \"DISENGAGE IMMEDIATELY\" take-over alarm easier to miss.</b> Default is Auto, which matches the alarm to cabin noise. Increase it when the cabin is loud, and ease it back only while the alarm stays impossible to ignore.<br><br><i><b>Disclaimer</b>: set it too quiet and you may not hear a take-over demand in time. The slider cannot go below 25%.</i>"), ""},

    {"CustomAlerts", tr("FrogPilot Alerts"), tr("<b>Optional FrogPilot sound alerts, plus louder or alternate cues, for selected driving events.</b>"), "../../frogpilot/assets/toggle_icons/icon_green_light.png"},
    {"GoatScream", tr("Goat Scream"), tr("<b>Replaces the standard \"Turn Exceeds Steering Limit\" chime with the infamous \"Goat Scream\" sound when openpilot can't turn as sharply as the road requires.</b> Turn it on if the novelty cue grabs your attention better or you like to live life on the edge. Leave it off for the standard alert."), ""},
    {"GreenLightAlert", tr("Green Light Alert"), tr("<b>Play an alert when the model predicts a red light ahead has turned green.</b> Turn this on if you want a reminder to look up at lights. Leave it off if you find the extra chime distracting.<br><br><i><b>Disclaimer</b>: openpilot does not explicitly detect traffic lights. This alert rides end-to-end model predictions from camera input and may trigger even when the light has not changed.</i>"), ""},
    {"LeadDepartingAlert", tr("Lead Departing Alert"), tr("<b>Plays a chime when the vehicle ahead pulls away from a stop.</b> Turn this on if you tend to miss the car in front moving at lights or in traffic. Leave it off if you find the reminder unnecessary."), ""},
    {"LoudBlindspotAlert", tr("Loud Blindspot Alert"), tr("<b>Plays a louder \"Car Detected in Blindspot\" warning when you start a lane change toward a vehicle in your blind spot.</b> Turn this on if you miss the normal chime in a noisy cabin, and off if you find the louder alert startling."), ""},
    {"SpeedLimitChangedAlert", tr("Speed Limit Changed Alert"), tr("<b>Play an alert whenever the detected posted speed limit changes.</b> Turn it on to catch new limits without watching the screen. Leave it off if the chime gets annoying on roads where the limit changes often."), ""}
  };

  for (const auto &[param, title, desc, icon] : soundsToggles) {
    AbstractControl *soundsToggle;

    if (param == "AlertVolumeControl") {
      FrogPilotManageControl *alertVolumeControlToggle = new FrogPilotManageControl(param, title, desc, icon);
      QObject::connect(alertVolumeControlToggle, &FrogPilotManageControl::manageButtonClicked, [soundsLayout, alertVolumeControlPanel]() {
        soundsLayout->setCurrentWidget(alertVolumeControlPanel);
      });
      soundsToggle = alertVolumeControlToggle;
    } else if (alertVolumeControlKeys.contains(param)) {
      std::map<float, QString> volumeLabels;
      for (int i = 0; i <= 101; ++i) {
        volumeLabels[i] = i == 0 ? tr("Muted") : i == 101 ? tr("Auto") : QString::number(i) + "%";
      }
      std::vector<QString> alertButton{tr("Test")};
      if (param == "WarningImmediateVolume" || param == "WarningSoftVolume") {
        soundsToggle = new FrogPilotParamValueButtonControl(param, title, desc, icon, 25, 101, QString(), volumeLabels, 1, true, {}, alertButton, false, false);
      } else {
        soundsToggle = new FrogPilotParamValueButtonControl(param, title, desc, icon, 0, 101, QString(), volumeLabels, 1, true, {}, alertButton, false, false);
      }

    } else if (param == "CustomAlerts") {
      FrogPilotManageControl *customAlertsToggle = new FrogPilotManageControl(param, title, desc, icon);
      QObject::connect(customAlertsToggle, &FrogPilotManageControl::manageButtonClicked, [soundsLayout, customAlertsPanel]() {
        soundsLayout->setCurrentWidget(customAlertsPanel);
      });
      soundsToggle = customAlertsToggle;

    } else {
      soundsToggle = new ParamControl(param, title, desc, icon);
    }

    toggles[param] = soundsToggle;

    if (alertVolumeControlKeys.contains(param)) {
      alertVolumeControlList->addItem(soundsToggle);
    } else if (customAlertsKeys.contains(param)) {
      customAlertsList->addItem(soundsToggle);
    } else {
      soundsList->addItem(soundsToggle);

      parentKeys.insert(param);
    }

    if (FrogPilotManageControl *frogPilotManageToggle = qobject_cast<FrogPilotManageControl*>(soundsToggle)) {
      QObject::connect(frogPilotManageToggle, &FrogPilotManageControl::manageButtonClicked, [this]() {
        emit openSubPanel();
        openDescriptions(forceOpenDescriptions, toggles);
      });
    }

    QObject::connect(soundsToggle, &AbstractControl::hideDescriptionEvent, [this]() {
      update();
    });
    QObject::connect(soundsToggle, &AbstractControl::showDescriptionEvent, [this]() {
      update();
    });
  }

  for (const QString &key : alertVolumeControlKeys) {
    FrogPilotParamValueButtonControl *toggle = static_cast<FrogPilotParamValueButtonControl*>(toggles[key]);
    QObject::connect(toggle, &FrogPilotParamValueButtonControl::buttonClicked, [key, toggle, this]() {
      toggle->updateParam();

      updateFrogPilotToggles();

      util::sleep_for(UI_FREQ);

      QString keyWithoutVolume = key;
      keyWithoutVolume.remove("Volume");

      QString camelCaseAlert = keyWithoutVolume;
      camelCaseAlert[0] = camelCaseAlert[0].toLower();

      QString snakeCaseAlert;
      for (int i = 0; i < keyWithoutVolume.size(); ++i) {
        QChar c = keyWithoutVolume[i];
        if (c.isUpper() && i > 0) {
          snakeCaseAlert += '_';
        }
        snakeCaseAlert += c.toLower();
      }

      if (started) {
        params_memory.put("TestAlert", camelCaseAlert.toStdString());
      } else {
        std::thread([key, snakeCaseAlert, this]() {
          playSound(snakeCaseAlert, params.getInt(key.toStdString()));
        }).detach();
      }
    });
  }

  QObject::connect(parent, &FrogPilotSettingsWindow::closeSubPanel, [soundsLayout, soundsPanel, this] {
    openDescriptions(forceOpenDescriptions, toggles);
    soundsLayout->setCurrentWidget(soundsPanel);
  });
  QObject::connect(uiState(), &UIState::uiUpdate, this, &FrogPilotSoundsPanel::updateState);

  for (auto &[key, toggle] : toggles) {
    if (alertVolumeControlKeys.contains(key)) {
      toggle->setVisible(true);
    }
  }

  updateToggles();
}

void FrogPilotSoundsPanel::showEvent(QShowEvent *event) {
  frogpilotToggleLevels = parent->frogpilotToggleLevels;

  updateToggles();
}

void FrogPilotSoundsPanel::updateState(const UIState &s) {
  if (!isVisible()) {
    return;
  }

  started = s.scene.started;
}

void FrogPilotSoundsPanel::updateToggles() {
  for (auto &[key, toggle] : toggles) {
    if (parentKeys.contains(key)) {
      toggle->setVisible(false);
    }
  }

  for (auto &[key, toggle] : toggles) {
    if (parentKeys.contains(key)) {
      continue;
    }

    bool setVisible = parent->tuningLevel >= frogpilotToggleLevels[key].toDouble();

    if (key == "LoudBlindspotAlert") {
      setVisible &= parent->hasBSM;
    }

    else if (key == "SpeedLimitChangedAlert") {
      setVisible &= params.getBool("ShowSpeedLimits") || (parent->hasOpenpilotLongitudinal && params.getBool("SpeedLimitController"));
    }

    toggle->setVisible(setVisible);

    if (setVisible) {
      if (alertVolumeControlKeys.contains(key)) {
        toggles["AlertVolumeControl"]->setVisible(true);
      } else if (customAlertsKeys.contains(key)) {
        toggles["CustomAlerts"]->setVisible(true);
      }
    }
  }

  openDescriptions(forceOpenDescriptions, toggles);

  update();
}
