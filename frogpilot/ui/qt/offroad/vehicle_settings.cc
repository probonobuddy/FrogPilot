#include <QRegularExpression>
#include <QTextStream>

#include "frogpilot/ui/qt/offroad/vehicle_settings.h"

QStringList getCarNames(const QString &carMake, QMap<QString, QString> &carModels) {
  static QMap<QString, QString> makeMap = {
    {"acura", "honda"},
    {"audi", "volkswagen"},
    {"buick", "gm"},
    {"cadillac", "gm"},
    {"chevrolet", "gm"},
    {"chrysler", "chrysler"},
    {"cupra", "volkswagen"},
    {"dodge", "chrysler"},
    {"ford", "ford"},
    {"genesis", "hyundai"},
    {"gmc", "gm"},
    {"holden", "gm"},
    {"honda", "honda"},
    {"hyundai", "hyundai"},
    {"jeep", "chrysler"},
    {"kia", "hyundai"},
    {"lexus", "toyota"},
    {"lincoln", "ford"},
    {"man", "volkswagen"},
    {"mazda", "mazda"},
    {"nissan", "nissan"},
    {"ram", "chrysler"},
    {"seat", "volkswagen"},
    {"škoda", "volkswagen"},
    {"subaru", "subaru"},
    {"tesla", "tesla"},
    {"toyota", "toyota"},
    {"volkswagen", "volkswagen"}
  };

  QStringList carNameList;

  QFile valuesFile(QString("../car/%1/values.py").arg(makeMap.value(carMake, carMake)));
  if (!valuesFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
    return carNameList;
  }

  QString fileContent = QTextStream(&valuesFile).readAll();
  valuesFile.close();

  fileContent.remove(QRegularExpression("#[^\n]*"));
  fileContent.remove(QRegularExpression("footnotes=\\[[^\\]]*\\],\\s*"));

  static QRegularExpression carNameRegex("CarDocs\\(\\s*\"([^\"]+)\"[^)]*\\)");
  static QRegularExpression platformRegex("((\\w+)\\s*=\\s*\\w+\\s*\\(\\s*\\[([\\s\\S]*?)\\]\\s*,)");
  static QRegularExpression validNameRegex("^[A-Za-z0-9 \u0160.()-]+$");

  QRegularExpressionMatchIterator platformMatches = platformRegex.globalMatch(fileContent);
  while (platformMatches.hasNext()) {
    QRegularExpressionMatch platformMatch = platformMatches.next();
    QString platformName = platformMatch.captured(2);
    QString platformSection = platformMatch.captured(3);

    QRegularExpressionMatchIterator carNameMatches = carNameRegex.globalMatch(platformSection);
    while (carNameMatches.hasNext()) {
      QString carName = carNameMatches.next().captured(1);

      if (carName.contains(validNameRegex) && carName.count(" ") >= 1) {
        QString firstWord = carName.section(" ", 0, 0);

        if (firstWord.compare(carMake, Qt::CaseInsensitive) == 0) {
          carModels[carName] = platformName;
          carNameList.append(carName);
        }
      }
    }
  }

  carNameList.sort();
  return carNameList;
}

FrogPilotVehiclesPanel::FrogPilotVehiclesPanel(FrogPilotSettingsWindow *parent) : FrogPilotListWidget(parent), parent(parent) {
  QJsonObject shownDescriptions = QJsonDocument::fromJson(QString::fromStdString(params.get("ShownToggleDescriptions")).toUtf8()).object();
  QString className = this->metaObject()->className();

  if (!shownDescriptions.value(className).toBool(false)) {
    forceOpenDescriptions = true;
    shownDescriptions.insert(className, true);
    params.put("ShownToggleDescriptions", QJsonDocument(shownDescriptions).toJson(QJsonDocument::Compact).toStdString());
  }

  QStackedLayout *vehiclesLayout = new QStackedLayout();
  addItem(vehiclesLayout);

  FrogPilotListWidget *settingsList = new FrogPilotListWidget(this);

  ScrollView *vehiclesPanel = new ScrollView(settingsList, this);

  vehiclesLayout->addWidget(vehiclesPanel);

  QStringList makes = {
    "Acura", "Audi", "Buick", "Cadillac", "Chevrolet", "Chrysler",
    "CUPRA", "Dodge", "Ford", "Genesis", "GMC", "Holden", "Honda",
    "Hyundai", "Jeep", "Kia", "Lexus", "Lincoln", "MAN", "Mazda",
    "Nissan", "Ram", "SEAT", "Škoda", "Subaru", "Tesla", "Toyota",
    "Volkswagen"
  };

  ButtonControl *selectMakeButton = new ButtonControl(tr("Car Make"), tr("SELECT"));
  QObject::connect(selectMakeButton, &ButtonControl::clicked, [makes, selectMakeButton, this]() {
    QString makeSelection = MultiOptionDialog::getSelection(tr("Choose your car make"), makes, "", this);
    if (!makeSelection.isEmpty()) {
      params.put("CarMake", makeSelection.toStdString());
      selectMakeButton->setValue(makeSelection);
    }
  });
  settingsList->addItem(selectMakeButton);

  ButtonControl *selectModelButton = new ButtonControl(tr("Car Model"), tr("SELECT"));
  QObject::connect(selectModelButton, &ButtonControl::clicked, [selectModelButton, this]() {
    QString modelSelection = MultiOptionDialog::getSelection(tr("Choose your car model"), getCarNames(QString::fromStdString(params.get("CarMake")).toLower(), carModels), "", this);
    if (!modelSelection.isEmpty()) {
      params.put("CarModel", carModels.value(modelSelection).toStdString());
      params.put("CarModelName", modelSelection.toStdString());
      selectModelButton->setValue(modelSelection);
    }
  });
  settingsList->addItem(selectModelButton);

  forceFingerprint = new ParamControl("ForceFingerprint", tr("Disable Automatic Fingerprint Detection"), tr("<b>Locks openpilot to the car you selected above instead of identifying it automatically each drive.</b> Turn this ON only if auto-detection keeps loading the wrong car. Leave it OFF to let openpilot detect your car each drive.<br><br><i><b>Disclaimer</b>: if the selected car is wrong, openpilot may steer, brake, or accelerate using the wrong vehicle profile.</i>"), "");
  settingsList->addItem(forceFingerprint);

  disableOpenpilotLong = new ParamControl("DisableOpenpilotLongitudinal", tr("Disable openpilot Longitudinal"), tr("<b>openpilot stops controlling the gas and brakes, handing speed and following distance back to your car's stock adaptive cruise.</b> openpilot keeps steering. Turn this ON if you prefer the car's factory cruise over openpilot's, or to troubleshoot longitudinal issues. Leave it OFF to let openpilot manage acceleration and braking.<br><br><i><b>Disclaimer</b>: openpilot will no longer brake or accelerate for you. How your car's stock cruise follows, stops, and resumes may behave very differently.</i>"), "");
  QObject::connect(disableOpenpilotLong, &ToggleControl::toggleFlipped, [parent, this](bool state) {
    if (state) {
      if (FrogPilotConfirmationDialog::yesorno(tr("Are you sure you want to completely disable openpilot longitudinal control?"), this)) {
        if (started) {
          if (FrogPilotConfirmationDialog::toggleReboot(this)) {
            Hardware::reboot();
          }
        }
      } else {
        params.putBool("DisableOpenpilotLongitudinal", false);
        disableOpenpilotLong->refresh();
      }
    }

    parent->updateVariables();
    updateToggles();
  });
  settingsList->addItem(disableOpenpilotLong);

  FrogPilotListWidget *gmList = new FrogPilotListWidget(this);
  FrogPilotListWidget *hkgList = new FrogPilotListWidget(this);
  FrogPilotListWidget *hondaList = new FrogPilotListWidget(this);
  FrogPilotListWidget *subaruList = new FrogPilotListWidget(this);
  FrogPilotListWidget *toyotaList = new FrogPilotListWidget(this);
  FrogPilotListWidget *vehicleInfoList = new FrogPilotListWidget(this);

  ScrollView *gmPanel = new ScrollView(gmList, this);
  ScrollView *hkgPanel = new ScrollView(hkgList, this);
  ScrollView *hondaPanel = new ScrollView(hondaList, this);
  ScrollView *subaruPanel = new ScrollView(subaruList, this);
  ScrollView *toyotaPanel = new ScrollView(toyotaList, this);
  ScrollView *vehicleInfoPanel = new ScrollView(vehicleInfoList, this);

  vehiclesLayout->addWidget(gmPanel);
  vehiclesLayout->addWidget(hkgPanel);
  vehiclesLayout->addWidget(hondaPanel);
  vehiclesLayout->addWidget(subaruPanel);
  vehiclesLayout->addWidget(toyotaPanel);
  vehiclesLayout->addWidget(vehicleInfoPanel);

  std::vector<std::tuple<QString, QString, QString, QString>> vehicleToggles {
    {"GMToggles", tr("General Motors Settings"), tr("<b>FrogPilot features for General Motors vehicles.</b>"), ""},
    {"ExperimentalGMTune", tr("FrogsGoMoo's Experimental Tune"), tr("<b>An experimental GM longitudinal tune from FrogsGoMoo that aims to smooth out the last moments of braking to a stop and the pull-away when you take off again.</b> Turn it on if stock stop-and-go feels rough or jerky, and turn it back off if it feels worse, since it is experimental and unproven."), ""},
    {"LongPitch", tr("Smooth Pedal Response on Hills"), tr("<b>Uses the road's grade to keep acceleration and braking steady on hills, instead of sagging on climbs or coasting on descents.</b> Leave this on if longitudinal control feels right on grades. Turn it off only if hills feel worse or less predictable.<br><br>Default: On."), ""},
    {"VoltSNG", tr("Stop-and-Go Hack"), tr("<b>Forces stop-and-go on the 2017 Chevy Volt, so openpilot pulls away from a stop without a gas tap.</b> Turn this ON only if you accept automatic pull-away from stops. Leave it OFF to resume manually.<br><br><i><b>Disclaimer</b>: the Volt has no stock stop-and-go, so this unofficial hack can make the car start moving from a stop when you do not expect it.</i>"), ""},

    {"HKGToggles", tr("Hyundai/Kia/Genesis Settings"), tr("<b>FrogPilot features for Genesis, Hyundai, and Kia vehicles.</b>"), ""},
    {"NewLongAPI", tr("comma's New Longitudinal API"), tr("<b>Uses comma's newer gas and brake control on Genesis, Hyundai, and Kia vehicles.</b> The default is right for almost every car, so turn it OFF only if acceleration or braking feels jerky or unstable, which reverts to the older control path.<br><br>Default: ON.<br><br><i><b>Disclaimer</b>: this changes how openpilot accelerates and brakes, so on some Genesis, Hyundai, and Kia models the car can accelerate or brake jerkily or unstably.</i>"), ""},
    {"TacoTuneHacks", tr("\"Taco Bell Run\" Steering Boost"), tr("<b>Boosts openpilot's steering force at low speeds so tight, slow turns pull harder.</b> Turn ON if low-speed turns (parking lots, U-turns, sharp corners) feel weak and openpilot under-steers. Leave OFF to keep the stock low-speed steering-torque limit.<br><br><i><b>Disclaimer</b>: this raises the panda steering-torque safety limit at low speed, so openpilot can yank the wheel harder and faster than stock allows.</i>"), ""},

    {"HondaToggles", tr("Acura/Honda Settings"), tr("<b>FrogPilot features for Acura and Honda vehicles.</b>"), ""},
    {"HondaAltTune", tr("Gentle Following"), tr("<b>Softens openpilot's acceleration and braking when following a lead vehicle on Honda Nidec vehicles.</b> Turn it on if the gas and brakes feel jerky in stop-and-go traffic. Leave it off if you want openpilot to react more quickly and hold the gap more tightly."), ""},
    {"HondaMaxBrake", tr("Increased Braking Force"), tr("<b>Lets openpilot brake harder on Honda Nidec vehicles, reaching stronger braking sooner for quicker, firmer stops.</b> Turn this ON if openpilot feels too soft or slow to stop. Leave it OFF if you prefer gentler, smoother braking.<br><br><i><b>Disclaimer</b>: this makes openpilot brake more abruptly and hold near-maximum braking force, so stops can feel sudden and grabby.</i>"), ""},
    {"HondaLowSpeedPedal", tr("Stronger Launch From Stop"), tr("<b>Removes openpilot's low-speed throttle limit so the car accelerates harder off the line through the comma pedal.</b> Turn ON if launches feel sluggish from a stop. Leave OFF for gentler, more controlled starts.<br><br><i><b>Disclaimer</b>: openpilot normally softens throttle at low speed because full pedal from a standstill is very aggressive, so the car may lunge forward harder than expected.</i>"), ""},

    {"SubaruToggles", tr("Subaru Settings"), tr("<b>FrogPilot features for Subaru vehicles.</b>"), ""},
    {"SubaruSNG", tr("Stop-and-Go"), tr("<b>openpilot can start moving from a full stop on its own when the lead pulls away, without a resume tap, on supported Subarus.</b> Leave this ON for automatic pull-away in stop-and-go traffic. Turn it OFF to resume manually.<br><br>Default: On.<br><br><i><b>Disclaimer</b>: the car can move from a full stop when you do not expect it.</i>"), ""},

    {"ToyotaToggles", tr("Toyota/Lexus Settings"), tr("<b>FrogPilot features for Toyota and Lexus vehicles.</b>"), ""},
    {"ToyotaDoors", tr("Automatically Lock/Unlock Doors"), tr("<b>Automatically lock or unlock the doors when shifting in and out of drive.</b><br><br>- \"Lock\": Locks the doors when you shift out of park.<br>- \"Unlock\": Unlocks the doors when you shift back to park."), ""},
    {"ClusterOffset", tr("Dashboard Speed Multiplier"), tr("<b>Multiplies the speed openpilot shows on screen so it matches your dashboard.</b> Raise it if openpilot reads below the dash. Lower it if it reads above, since this only changes the displayed speed.<br><br>Default: 1.015x."), ""},
    {"ToyotaDSUBypass", tr("Driving Support Unit Bypass"), tr("<b>Hands gas and brake control to openpilot on a \"Toyota Safety Sense P\" (TSS-P) car using a wired Driving Support Unit (DSU) bypass harness.</b> Turn it off if you haven't installed the bypass harness, to keep stock cruise.<br><br>Default: On.<br><br><i><b>Disclaimer</b>: without the harness installed, openpilot can fail to accelerate or brake when expected.</i>"), ""},
    {"FrogsGoMoosTweak", tr("FrogsGoMoo's Longitudinal Tune"), tr("<b>Applies FrogsGoMoo's personal Toyota tuning for smoother braking into stops and quicker takeoffs from a stop.</b> Turn it on if you want gentler, less abrupt stops and a more eager launch. Leave it off to keep openpilot's stock stop-and-go feel."), ""},
    {"LockDoorsTimer", tr("Lock Doors On Ignition Off After"), tr("<b>How many seconds after you shut the car off openpilot waits before locking the doors on its own.</b> It only locks once the driver camera no longer sees a face and the doors are shut, so set a longer delay to give yourself time to grab your things and step out, or leave it on \"Never\" to keep auto-locking off.<br><br>Default: Never (off)."), ""},
    {"SNGHack", tr("Force Stop-and-Go"), tr("<b>openpilot can start moving from a complete stop without you pressing the gas or resume, on Toyota and Lexus models that lack factory stop-and-go.</b> Turn it ON for automatic pull-away when the lead moves. Leave it OFF to resume manually after each stop.<br><br>Default: On.<br><br><i><b>Disclaimer</b>: the car can move when you expect it to stay stopped, so stay ready to brake.</i>"), ""},

    {"VehicleInfo", tr("Vehicle Info"), tr("<b>What openpilot supports on your detected vehicle, including radar, blind spot, longitudinal, and stop-and-go.</b>"), ""},
    {"HardwareDetected", tr("3rd Party Hardware"), tr("<b>Lists any aftermarket hardware openpilot detected on your car, such as a comma Pedal, Smart DSU (SDSU), or Zorro Steering Sensor (ZSS).</b> \"None\" is normal and expected on a stock vehicle."), ""},
    {"BlindSpotSupport", tr("Blind Spot Support"), tr("<b>Whether openpilot can use your vehicle's factory blind spot monitoring data.</b> \"Yes\" means openpilot reads those built-in sensors to help detect vehicles beside you. \"No\" means no factory blind spot data is available to openpilot."), ""},
    {"PedalSupport", tr("comma Pedal Support"), tr("<b>Whether your car can use a \"comma pedal\" gas-pedal interceptor.</b> \"Yes\" means the accessory is supported on your vehicle. Most vehicles will show \"No\"."), ""},
    {"OpenpilotLongitudinal", tr("openpilot Longitudinal Support"), tr("<b>Whether openpilot controls this car's acceleration and braking instead of the car's stock adaptive cruise control.</b> Shows \"Yes\" if openpilot handles speed on your vehicle, \"No\" if the car's own system does."), ""},
    {"RadarSupport", tr("Radar Support"), tr("<b>Whether openpilot uses your car's radar alongside the camera to track lead vehicles.</b> \"Yes\" means radar and camera are fused for more reliable lead detection. \"No\" means openpilot tracks leads from the camera alone."), ""},
    {"SDSUSupport", tr("Smart Driving Support Unit (SDSU)"), tr("<b>Shows whether your car can use a \"Smart Driving Support Unit\" (SDSU), a third-party module that lets openpilot control gas and brakes on older Toyota and Lexus models.</b> \"Yes\" means the module is compatible with your car. \"No\" means it is not needed or not supported."), ""},
    {"SNGSupport", tr("Stop-and-Go Support"), tr("<b>Shows whether your car can stop and pull away on its own in traffic.</b> \"Yes\" means openpilot follows a lead to a full stop and resumes automatically. \"No\" means you tap the gas or resume to move off after stopping."), ""}
  };

  for (const auto &[param, title, desc, icon] : vehicleToggles) {
    AbstractControl *vehicleToggle;

    if (param == "GMToggles") {
      ButtonControl *gmButton = new ButtonControl(title, tr("MANAGE"), desc);
      QObject::connect(gmButton, &ButtonControl::clicked, [vehiclesLayout, gmPanel, this]() {
        openDescriptions(forceOpenDescriptions, toggles);
        vehiclesLayout->setCurrentWidget(gmPanel);
      });
      vehicleToggle = gmButton;

    } else if (param == "HKGToggles") {
      ButtonControl *hkgButton = new ButtonControl(title, tr("MANAGE"), desc);
      QObject::connect(hkgButton, &ButtonControl::clicked, [vehiclesLayout, hkgPanel, this]() {
        openDescriptions(forceOpenDescriptions, toggles);
        vehiclesLayout->setCurrentWidget(hkgPanel);
      });
      vehicleToggle = hkgButton;

    } else if (param == "HondaToggles") {
      ButtonControl *hondaButton = new ButtonControl(title, tr("MANAGE"), desc);
      QObject::connect(hondaButton, &ButtonControl::clicked, [vehiclesLayout, hondaPanel, this]() {
        openDescriptions(forceOpenDescriptions, toggles);
        vehiclesLayout->setCurrentWidget(hondaPanel);
      });
      vehicleToggle = hondaButton;

    } else if (param == "SubaruToggles") {
      ButtonControl *subaruButton = new ButtonControl(title, tr("MANAGE"), desc);
      QObject::connect(subaruButton, &ButtonControl::clicked, [vehiclesLayout, subaruPanel, this]() {
        openDescriptions(forceOpenDescriptions, toggles);
        vehiclesLayout->setCurrentWidget(subaruPanel);
      });
      vehicleToggle = subaruButton;

    } else if (param == "ToyotaToggles") {
      ButtonControl *toyotaButton = new ButtonControl(title, tr("MANAGE"), desc);
      QObject::connect(toyotaButton, &ButtonControl::clicked, [vehiclesLayout, toyotaPanel, this]() {
        openDescriptions(forceOpenDescriptions, toggles);
        vehiclesLayout->setCurrentWidget(toyotaPanel);
      });
      vehicleToggle = toyotaButton;
    } else if (param == "ToyotaDoors") {
      std::vector<QString> lockToggles{"LockDoors", "UnlockDoors"};
      std::vector<QString> lockToggleNames{tr("Lock"), tr("Unlock")};
      vehicleToggle = new FrogPilotButtonToggleControl(param, title, desc, icon, lockToggles, lockToggleNames);
    } else if (param == "LockDoorsTimer") {
      std::map<float, QString> autoLockLabels;
      for (int i = 0; i <= 300; ++i) {
        autoLockLabels[i] = i == 0 ? tr("Never") : QString::number(i) + tr(" seconds");
      }
      vehicleToggle = new FrogPilotParamValueControl(param, title, desc, icon, 0, 300, QString(), autoLockLabels, 5);
    } else if (param == "ClusterOffset") {
      std::vector<QString> clusterOffsetButton{"Reset"};
      FrogPilotParamValueButtonControl *clusterOffsetToggle = new FrogPilotParamValueButtonControl(param, title, desc, icon, 1.000, 1.050, "x", std::map<float, QString>(), 0.001, false, {}, clusterOffsetButton, false, false);
      QObject::connect(clusterOffsetToggle, &FrogPilotParamValueButtonControl::buttonClicked, [clusterOffsetToggle, this]() {
        params.putFloat("ClusterOffset", params_default.getFloat("ClusterOffset"));
        clusterOffsetToggle->refresh();
      });
      vehicleToggle = clusterOffsetToggle;

    } else if (param == "VehicleInfo") {
      ButtonControl *VehicleInfoButton = new ButtonControl(title, tr("VIEW"), desc);
      QObject::connect(VehicleInfoButton, &ButtonControl::clicked, [vehiclesLayout, vehicleInfoPanel, this]() {
        openDescriptions(forceOpenDescriptions, toggles);
        vehiclesLayout->setCurrentWidget(vehicleInfoPanel);
      });
      vehicleToggle = VehicleInfoButton;
    } else if (vehicleInfoKeys.contains(param)) {
      vehicleToggle = new LabelControl(title, "", desc);

    } else {
      vehicleToggle = new ParamControl(param, title, desc, icon);
    }

    toggles[param] = vehicleToggle;

    if (gmKeys.contains(param)) {
      gmList->addItem(vehicleToggle);
    } else if (hkgKeys.contains(param)) {
      hkgList->addItem(vehicleToggle);
    } else if (hondaKeys.contains(param)) {
      hondaList->addItem(vehicleToggle);
    } else if (subaruKeys.contains(param)) {
      subaruList->addItem(vehicleToggle);
    } else if (toyotaKeys.contains(param)) {
      toyotaList->addItem(vehicleToggle);
    } else if (vehicleInfoKeys.contains(param)) {
      vehicleInfoList->addItem(vehicleToggle);
    } else {
      settingsList->addItem(vehicleToggle);

      parentKeys.insert(param);
    }

    if (ButtonControl *buttonControl = qobject_cast<ButtonControl*>(vehicleToggle)) {
      QObject::connect(buttonControl, &ButtonControl::clicked, this, &FrogPilotVehiclesPanel::openSubPanel);
    }

    QObject::connect(vehicleToggle, &AbstractControl::hideDescriptionEvent, [this]() {
      update();
    });
    QObject::connect(vehicleToggle, &AbstractControl::showDescriptionEvent, [this]() {
      update();
    });
  }

  static_cast<FrogPilotParamValueControl*>(toggles["LockDoorsTimer"])->setWarning(tr("<b>Warning:</b> openpilot can't tell whether the keys are still inside, so keep a spare key to avoid getting locked out."));

  QSet<QString> rebootKeys = {"HondaAltTune", "NewLongAPI", "TacoTuneHacks", "ToyotaDSUBypass"};
  for (const QString &key : rebootKeys) {
    QObject::connect(static_cast<ToggleControl*>(toggles[key]), &ToggleControl::toggleFlipped, [key, this](bool state) {
      if (started) {
        if (key == "HondaAltTune" || key == "TacoTuneHacks" && state) {
          if (FrogPilotConfirmationDialog::toggleReboot(this)) {
            Hardware::reboot();
          }
        } else if (key != "TacoTuneHacks") {
          if (FrogPilotConfirmationDialog::toggleReboot(this)) {
            Hardware::reboot();
          }
        }
      }
    });
  }

  openDescriptions(forceOpenDescriptions, toggles);

  QObject::connect(uiState(), &UIState::offroadTransition, [selectMakeButton, selectModelButton, this]() {
    std::thread([selectMakeButton, selectModelButton, this]() {
      selectMakeButton->setValue(QString::fromStdString(params.get("CarMake", true)));
      selectModelButton->setValue(QString::fromStdString(params.get(params.get("CarModelName").empty() ? "CarModel" : "CarModelName")));
    }).detach();
  });

  QObject::connect(parent, &FrogPilotSettingsWindow::closeSubPanel, [vehiclesLayout, vehiclesPanel, this] {
    if (forceOpenDescriptions) {
      openDescriptions(forceOpenDescriptions, toggles);

      disableOpenpilotLong->showDescription();
      forceFingerprint->showDescription();
    }
    vehiclesLayout->setCurrentWidget(vehiclesPanel);
  });
  QObject::connect(uiState(), &UIState::uiUpdate, this, &FrogPilotVehiclesPanel::updateState);
}

void FrogPilotVehiclesPanel::showEvent(QShowEvent *event) {
  if (forceOpenDescriptions) {
    disableOpenpilotLong->showDescription();
    forceFingerprint->showDescription();
  }

  frogpilotToggleLevels = parent->frogpilotToggleLevels;

  QStringList detected;
  if (parent->hasPedal) detected << "comma Pedal";
  if (parent->hasSDSU) detected << "SDSU";
  if (parent->hasZSS) detected << "ZSS";
  static_cast<LabelControl*>(toggles["HardwareDetected"])->setText(detected.isEmpty() ? tr("None") : detected.join(", "));

  static_cast<LabelControl*>(toggles["BlindSpotSupport"])->setText(parent->hasBSM ? tr("Yes") : tr("No"));
  static_cast<LabelControl*>(toggles["OpenpilotLongitudinal"])->setText(parent->hasOpenpilotLongitudinal ? tr("Yes") : tr("No"));
  static_cast<LabelControl*>(toggles["PedalSupport"])->setText(parent->canUsePedal ? tr("Yes") : tr("No"));
  static_cast<LabelControl*>(toggles["RadarSupport"])->setText(parent->hasRadar ? tr("Yes") : tr("No"));
  static_cast<LabelControl*>(toggles["SDSUSupport"])->setText(parent->canUseSDSU ? tr("Yes") : tr("No"));
  static_cast<LabelControl*>(toggles["SNGSupport"])->setText(parent->hasSNG ? tr("Yes") : tr("No"));

  updateToggles();
}

void FrogPilotVehiclesPanel::updateState(const UIState &s) {
  if (!isVisible()) {
    return;
  }

  started = s.scene.started;
}

void FrogPilotVehiclesPanel::updateToggles() {
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

    if (gmKeys.contains(key)) {
      setVisible &= parent->isGM;
    } else if (hkgKeys.contains(key)) {
      setVisible &= parent->isHKG;
    } else if (hondaKeys.contains(key)) {
      setVisible &= parent->isHonda;
    } else if (subaruKeys.contains(key)) {
      setVisible &= parent->isSubaru;
    } else if (toyotaKeys.contains(key)) {
      setVisible &= parent->isToyota;
    } else if (vehicleInfoKeys.contains(key)) {
      setVisible = true;
    }

    if (longitudinalKeys.contains(key)) {
      setVisible &= parent->hasOpenpilotLongitudinal;
    }

    if (key == "HondaAltTune") {
      setVisible &= parent->isHondaNidec;
    }

    else if (key == "HondaLowSpeedPedal") {
      setVisible &= parent->hasPedal;
    }

    else if (key == "HondaMaxBrake") {
      setVisible &= parent->isHondaNidec;
    }

    else if (key == "SNGHack") {
      setVisible &= !parent->hasPedal && !parent->hasSNG;
    }

    else if (key == "SubaruSNG") {
      setVisible &= parent->hasSNG;
    }

    else if (key == "TacoTuneHacks") {
      setVisible &= parent->isHKGCanFd;
    }

    else if (key == "ToyotaDSUBypass") {
      setVisible &= parent->canUseSDSU && !parent->hasSDSU;
    }

    else if (key == "VoltSNG") {
      setVisible &= parent->isVolt && !parent->hasSNG;
    }

    toggle->setVisible(setVisible);

    if (setVisible) {
      if (gmKeys.contains(key)) {
        toggles["GMToggles"]->setVisible(true);
      } else if (hkgKeys.contains(key)) {
        toggles["HKGToggles"]->setVisible(true);
      } else if (hondaKeys.contains(key)) {
        toggles["HondaToggles"]->setVisible(true);
      } else if (subaruKeys.contains(key)) {
        toggles["SubaruToggles"]->setVisible(true);
      } else if (toyotaKeys.contains(key)) {
        toggles["ToyotaToggles"]->setVisible(true);
      } else if (vehicleInfoKeys.contains(key)) {
        toggles["VehicleInfo"]->setVisible(true);
      }
    }
  }

  disableOpenpilotLong->setVisible((parent->hasOpenpilotLongitudinal || parent->openpilotLongitudinalControlDisabled) && !parent->hasExperimentalOpenpilotLongitudinal && parent->tuningLevel >= frogpilotToggleLevels["DisableOpenpilotLongitudinal"].toBool());
  forceFingerprint->setVisible(parent->tuningLevel >= frogpilotToggleLevels["ForceFingerprint"].toBool());

  openDescriptions(forceOpenDescriptions, toggles);

  update();
}
