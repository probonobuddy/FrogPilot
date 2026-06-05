#include "frogpilot/ui/screenrecorder/screenrecorder.h"
#include "frogpilot/ui/qt/offroad/device_settings.h"

FrogPilotDevicePanel::FrogPilotDevicePanel(FrogPilotSettingsWindow *parent) : FrogPilotListWidget(parent), parent(parent) {
  QJsonObject shownDescriptions = QJsonDocument::fromJson(QString::fromStdString(params.get("ShownToggleDescriptions")).toUtf8()).object();
  QString className = this->metaObject()->className();

  if (!shownDescriptions.value(className).toBool(false)) {
    forceOpenDescriptions = true;
    shownDescriptions.insert(className, true);
    params.put("ShownToggleDescriptions", QJsonDocument(shownDescriptions).toJson(QJsonDocument::Compact).toStdString());
  }

  ScreenRecorder *screenRecorder = new ScreenRecorder(this);
  screenRecorder->setVisible(false);

  QStackedLayout *deviceLayout = new QStackedLayout();
  addItem(deviceLayout);

  FrogPilotListWidget *deviceList = new FrogPilotListWidget(this);

  ScrollView *devicePanel = new ScrollView(deviceList, this);

  deviceLayout->addWidget(devicePanel);

  FrogPilotListWidget *deviceManagementList = new FrogPilotListWidget(this);
  FrogPilotListWidget *screenList = new FrogPilotListWidget(this);

  ScrollView *deviceManagementPanel = new ScrollView(deviceManagementList, this);
  ScrollView *screenPanel = new ScrollView(screenList, this);

  deviceLayout->addWidget(deviceManagementPanel);
  deviceLayout->addWidget(screenPanel);

  const std::vector<std::tuple<QString, QString, QString, QString>> deviceToggles {
    {"DeviceManagement", tr("Device Management"), tr("<b>Settings that control how the device runs, powers off, and manages driving data.</b>"), "../../frogpilot/assets/toggle_icons/icon_device.png"},
    {"DeviceShutdown", tr("Device Shutdown Timer"), tr("<b>How long the device stays powered on after you park before it shuts itself off.</b> A shorter time saves the car's battery. A longer time keeps the device awake and ready when you head back out soon.<br><br>Default: 6 hours."), ""},
    {"NoLogging", tr("Disable Logging"), tr("<b>Stops the device from recording your drives, so no logs or dashcam footage are saved to disk.</b> Turn it on to save storage and keep your drives private. Leave it off if you might want to review, replay, or report a drive later."), ""},
    {"NoUploads", tr("Disable Uploads"), tr("<b>Stop the device from uploading your drives to \"comma connect\".</b> Turn this on for privacy or to save data, but leave it off if you may need comma's debugging or official support, which both rely on uploaded drives.<br><br>- \"Disable Onroad Only\": keep uploads off while driving but still upload when parked on Wi-Fi."), ""},
    {"HigherBitrate", tr("High-Quality Recording"), tr("<b>Saves your drive footage at a higher bitrate for sharper video.</b> Turn it on when you want the clearest local recordings at the sacrifice of double the storage usage."), ""},
    {"LowVoltageShutdown", tr("Low-Voltage Cutoff"), tr("<b>While parked, shuts the device down once the car battery drops below this voltage, so it won't drain flat.</b> A higher value powers off sooner to protect the battery. A lower value keeps recording longer before it powers off. The default works for most setups.<br><br>Default: 11.8 volts."), ""},
    {"IncreaseThermalLimits", tr("Raise Temperature Limits"), tr("<b>Raises the temperature where openpilot throttles or shuts down, so the device can overheat instead of protecting itself.</b> Leave this off unless you accept permanent device damage to keep driving in heat.<br><br><i><b>Disclaimer</b>: this overrides a thermal safeguard, so the device can run past its intended temperature limit and suffer permanent damage or a shortened lifespan.</i>"), ""},
    {"FrogPilotTelemetry", tr("Share Driving Data"), tr("<b>Uploads scrubbed driving logs to FrogPilot to help improve existing and future driving features.</b> The log strips out video, location, route name, and your VIN before it leaves the device. Leave it on to help out, or turn it off if you'd rather not share your driving data.<br><br>Default: On."), ""},
    {"UseKonikServer", tr("Use Konik Server"), tr("<b>Uploads your driving data to Konik's server (\"connect.konik.ai\") instead of comma's (\"connect.comma.ai\"), so your routes show up on Konik's dashboard.</b> Turn it on if you review your routes with Konik's tools. Leave it off for comma's dashboard."), ""},

    {"ScreenManagement", tr("Screen Settings"), tr("<b>Control the screen's brightness, recording, tap timeouts, and standby behavior.</b>"), "../../frogpilot/assets/toggle_icons/icon_light.png"},
    {"ScreenBrightness", tr("Screen Brightness (Offroad)"), tr("<b>Sets how bright the screen is while parked.</b> Raise it if the parked display looks too dim, or lower it to ease glare and save power. Leave it on \"Auto\" for the normal offroad brightness.<br><br>Default: Auto."), ""},
    {"ScreenBrightnessOnroad", tr("Screen Brightness (Onroad)"), tr("<b>Sets how bright the screen is while driving.</b> At \"Screen Off\" the display fully blanks, hiding the path, lead car, and on-screen alerts. Leave it on \"Auto\" to match the ambient light, raise it in bright sun, or lower it to cut glare.<br><br>Default: Auto.<br><br><i><b>Disclaimer</b>: with the display blank while driving you can miss visual alerts and path context that require you to take over. Audible alerts still sound.</i>"), ""},
    {"ScreenRecorder", tr("Screen Recorder"), tr("<b>Adds a \"RECORD\" button to the driving screen so you can capture footage of what's on the display.</b> Turn it on if you want to record screen clips while driving. Leave it off to keep the driving screen uncluttered."), ""},
    {"ScreenTimeout", tr("Screen Timeout (Offroad)"), tr("<b>How long the screen stays on after a tap while parked, before it goes dark.</b> A longer timeout keeps the display lit while you sit in the car. A shorter one saves power and blanks the screen sooner.<br><br>Default: 30 seconds."), ""},
    {"ScreenTimeoutOnroad", tr("Screen Timeout (Onroad)"), tr("<b>How long the screen stays lit and interactive after you tap it while driving.</b> A longer timeout keeps the display awake after a touch. A shorter one dims back to the driving view sooner.<br><br>Default: 30 seconds."), ""},
    {"StandbyMode", tr("Standby Mode"), tr("<b>Blanks the screen while driving, waking it automatically for alerts, engaging or disengaging, or a tap.</b> Turn it on to cut screen wear and device heat. Leave it off if you want your speed and info visible at a glance the whole time."), ""}
  };

  for (const auto &[param, title, desc, icon] : deviceToggles) {
    AbstractControl *deviceToggle;

    if (param == "DeviceManagement") {
      FrogPilotManageControl *deviceManagementToggle = new FrogPilotManageControl(param, title, desc, icon);
      QObject::connect(deviceManagementToggle, &FrogPilotManageControl::manageButtonClicked, [deviceLayout, deviceManagementPanel]() {
        deviceLayout->setCurrentWidget(deviceManagementPanel);
      });
      deviceToggle = deviceManagementToggle;
    } else if (param == "DeviceShutdown") {
      std::map<float, QString> shutdownLabels;
      for (int i = 0; i <= 33; ++i) {
        shutdownLabels[i] = i == 0 ? tr("5 mins") : i <= 3 ? QString::number(i * 15) + tr(" mins") : QString::number(i - 3) + (i == 4 ? tr(" hour") : tr(" hours"));
      }
      deviceToggle = new FrogPilotParamValueControl(param, title, desc, icon, 0, 33, QString(), shutdownLabels, 1, true);
    } else if (param == "NoUploads") {
      std::vector<QString> uploadsToggles{"DisableOnroadUploads"};
      std::vector<QString> uploadsToggleNames{tr("Disable Onroad Only")};
      deviceToggle = new FrogPilotButtonToggleControl(param, title, desc, icon, uploadsToggles, uploadsToggleNames);
    } else if (param == "LowVoltageShutdown") {
      deviceToggle = new FrogPilotParamValueControl(param, title, desc, icon, 11.8, 12.5, tr(" volts"), std::map<float, QString>(), 0.1);

    } else if (param == "ScreenManagement") {
      FrogPilotManageControl *screenToggle = new FrogPilotManageControl(param, title, desc, icon);
      QObject::connect(screenToggle, &FrogPilotManageControl::manageButtonClicked, [deviceLayout, screenPanel]() {
        deviceLayout->setCurrentWidget(screenPanel);
      });
      deviceToggle = screenToggle;
    } else if (param == "ScreenBrightness" || param == "ScreenBrightnessOnroad") {
      std::map<float, QString> brightnessLabels;
      int minBrightness = (param == "ScreenBrightnessOnroad") ? 0 : 1;
      for (int i = 0; i <= 101; ++i) {
        brightnessLabels[i] = i == 0 ? tr("Screen Off") : i == 101 ? tr("Auto") : QString::number(i) + "%";
      }
      deviceToggle = new FrogPilotParamValueControl(param, title, desc, icon, minBrightness, 101, QString(), brightnessLabels, 1, true);
    } else if (param == "ScreenRecorder") {
      std::vector<QString> recorderButtonNames{tr("Start Recording"), tr("Stop Recording")};
      FrogPilotButtonControl *recorderToggle = new FrogPilotButtonControl(param, title, desc, icon, recorderButtonNames, true);
      QObject::connect(recorderToggle, &FrogPilotButtonControl::buttonClicked, [recorderToggle, screenRecorder](int id) {
        if (id == 0) {
          recorderToggle->setCheckedButton(1);

          recorderToggle->setVisibleButton(0, false);
          recorderToggle->setVisibleButton(1, true);

          screenRecorder->startRecording();
        } else if (id == 1) {
          recorderToggle->clearCheckedButtons(true);

          recorderToggle->setVisibleButton(0, true);
          recorderToggle->setVisibleButton(1, false);

          screenRecorder->stopRecording();
        }
      });
      recorderToggle->setVisibleButton(1, false);
      deviceToggle = recorderToggle;
    } else if (param == "ScreenTimeout" || param == "ScreenTimeoutOnroad") {
      deviceToggle = new FrogPilotParamValueControl(param, title, desc, icon, 5, 60, tr(" seconds"), {}, 5);

    } else {
      deviceToggle = new ParamControl(param, title, desc, icon);
    }

    toggles[param] = deviceToggle;

    if (deviceManagementKeys.contains(param)) {
      deviceManagementList->addItem(deviceToggle);
    } else if (screenKeys.contains(param)) {
      screenList->addItem(deviceToggle);
    } else {
      deviceList->addItem(deviceToggle);

      parentKeys.insert(param);
    }

    if (FrogPilotManageControl *frogPilotManageToggle = qobject_cast<FrogPilotManageControl*>(deviceToggle)) {
      QObject::connect(frogPilotManageToggle, &FrogPilotManageControl::manageButtonClicked, [this]() {
        emit openSubPanel();
        openDescriptions(forceOpenDescriptions, toggles);
      });
    }

    QObject::connect(deviceToggle, &AbstractControl::hideDescriptionEvent, [this]() {
      update();
    });
    QObject::connect(deviceToggle, &AbstractControl::showDescriptionEvent, [this]() {
      update();
    });
  }

  static_cast<ParamControl*>(toggles["FrogPilotTelemetry"])->setConfirmation(true, false);
  static_cast<ParamControl*>(toggles["IncreaseThermalLimits"])->setConfirmation(true, false);
  static_cast<ParamControl*>(toggles["NoLogging"])->setConfirmation(true, false);
  static_cast<ParamControl*>(toggles["NoUploads"])->setConfirmation(true, false);

  QSet<QString> brightnessKeys = {"ScreenBrightness", "ScreenBrightnessOnroad"};
  for (const QString &key : brightnessKeys) {
    FrogPilotParamValueControl *paramControl = static_cast<FrogPilotParamValueControl*>(toggles[key]);
    QObject::connect(paramControl, &FrogPilotParamValueControl::valueChanged, [key, this](int value) {
      if (!started && key == "ScreenBrightness") {
        Hardware::set_brightness(value);
      } else if (started && key == "ScreenBrightnessOnroad") {
        Hardware::set_brightness(value);
      }
    });
  }

  QSet<QString> forceUpdateKeys = {"NoUploads"};
  for (const QString &key : forceUpdateKeys) {
    QObject::connect(static_cast<FrogPilotButtonToggleControl*>(toggles[key]), &FrogPilotButtonToggleControl::buttonClicked, this, &FrogPilotDevicePanel::updateToggles);
    QObject::connect(static_cast<ToggleControl*>(toggles[key]), &ToggleControl::toggleFlipped, this, &FrogPilotDevicePanel::updateToggles);
  }

  QSet<QString> rebootKeys = {"HigherBitrate", "UseKonikServer"};
  for (const QString &key : rebootKeys) {
    QObject::connect(static_cast<ToggleControl*>(toggles[key]), &ToggleControl::toggleFlipped, [key, this](bool state) {
      QString filePath;
      if (key == "HigherBitrate") {
        filePath = "/cache/use_HD";
      } else if (key == "UseKonikServer") {
        filePath = "/cache/use_konik";
      }

      if (!filePath.isEmpty()) {
        QFile toggleFile(filePath);
        if (state) {
          if (!toggleFile.exists()) {
            toggleFile.open(QIODevice::WriteOnly);
            toggleFile.close();
          }
        } else {
          if (toggleFile.exists()) {
            toggleFile.remove();
          }
        }
      }

      if (FrogPilotConfirmationDialog::toggleReboot(this)) {
        Hardware::reboot();
      }
    });
  }

  openDescriptions(forceOpenDescriptions, toggles);

  QObject::connect(parent, &FrogPilotSettingsWindow::closeSubPanel, [deviceLayout, devicePanel, this] {
    openDescriptions(forceOpenDescriptions, toggles);
    deviceLayout->setCurrentWidget(devicePanel);
  });
  QObject::connect(uiState(), &UIState::uiUpdate, this, &FrogPilotDevicePanel::updateState);
}

void FrogPilotDevicePanel::showEvent(QShowEvent *event) {
  frogpilotToggleLevels = parent->frogpilotToggleLevels;

  updateToggles();
}

void FrogPilotDevicePanel::updateState(const UIState &s) {
  if (!isVisible()) {
    return;
  }

  started = s.scene.started;
}

void FrogPilotDevicePanel::updateToggles() {
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

    if (key == "HigherBitrate") {
      setVisible &= params.getBool("DeviceManagement") && params.getBool("NoUploads") && !params.getBool("DisableOnroadUploads");
    }

    else if (key == "UseKonikServer" && QFile("/data/not_vetted").exists()) {
      static_cast<ToggleControl*>(toggle)->forceOn(true);
    }

    toggle->setVisible(setVisible);

    if (setVisible) {
      if (deviceManagementKeys.contains(key)) {
        toggles["DeviceManagement"]->setVisible(true);
      } else if (screenKeys.contains(key)) {
        toggles["ScreenManagement"]->setVisible(true);
      }
    }
  }

  openDescriptions(forceOpenDescriptions, toggles);

  update();
}
