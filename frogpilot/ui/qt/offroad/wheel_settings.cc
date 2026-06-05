#include "frogpilot/ui/qt/offroad/wheel_settings.h"

FrogPilotWheelPanel::FrogPilotWheelPanel(FrogPilotSettingsWindow *parent) : FrogPilotListWidget(parent), parent(parent) {
  QJsonObject shownDescriptions = QJsonDocument::fromJson(QString::fromStdString(params.get("ShownToggleDescriptions")).toUtf8()).object();
  QString className = this->metaObject()->className();

  if (!shownDescriptions.value(className).toBool(false)) {
    forceOpenDescriptions = true;
    shownDescriptions.insert(className, true);
    params.put("ShownToggleDescriptions", QJsonDocument(shownDescriptions).toJson(QJsonDocument::Compact).toStdString());
  }

  const std::vector<std::tuple<QString, QString, QString, QString>> wheelToggles {
    {"DistanceButtonControl", tr("Distance Button"), tr("<b>A quick press of the car's \"Distance\" button runs the FrogPilot action you assign.</b> Some actions change how openpilot steers, accelerates, or brakes. Pick the action you want, or \"No Action\" to leave the button at its stock function. The speed-control actions need openpilot controlling speed.<br><br>Default: Change \"Personality Profile\"."), "../../frogpilot/assets/toggle_icons/icon_mute.png"},
    {"LongDistanceButtonControl", tr("Distance Button (Long Press)"), tr("<b>Holding the car's \"Distance\" button past 0.5 seconds runs the FrogPilot action you assign.</b> Some actions change how openpilot steers, accelerates, or brakes. Pick the action you want, or \"No Action\" to leave the long press unused. The speed-control actions need openpilot controlling speed.<br><br>Default: Toggle \"Experimental Mode\" On/Off."), "../../frogpilot/assets/toggle_icons/icon_mute.png"},
    {"VeryLongDistanceButtonControl", tr("Distance Button (Very Long Press)"), tr("<b>Holding the car's \"Distance\" button past 2.5 seconds runs this action and repeats whatever the long press is set to.</b> Some actions change how openpilot steers, accelerates, or brakes. Choosing \"No Action\" leaves only this very-long slot unused. The long-press action still fires once you pass 2.5 seconds.<br><br>Default: Toggle \"Traffic Mode\" On/Off."), "../../frogpilot/assets/toggle_icons/icon_mute.png"},
    {"LKASButtonControl", tr("LKAS Button"), tr("<b>Pressing the car's \"LKAS\" button runs the FrogPilot action you assign.</b> Some actions change how openpilot steers, accelerates, or brakes. Pick the action you want, or \"No Action\" to leave the button unused. Only \"No Action\" and \"Pause Steering\" work without openpilot controlling speed.<br><br>Default: Toggle \"Experimental Mode\" On/Off."), "../../frogpilot/assets/toggle_icons/icon_mute.png"}
  };

  for (const auto &[param, title, desc, icon] : wheelToggles) {
    QMap<int, QString> functionsMap {
      {0, tr("No Action")},
      {3, tr("Pause Steering")}
    };

    QMap<int, QString> longitudinalFunctionsMap {
      {1, tr("Change \"Personality Profile\"")},
      {2, tr("Force openpilot to Coast")},
      {4, tr("Pause Acceleration/Braking")},
      {5, tr("Toggle \"Experimental Mode\" On/Off")},
      {6, tr("Toggle \"Traffic Mode\" On/Off")}
    };

    ButtonControl *wheelToggle = new ButtonControl(title, tr("SELECT"), desc);
    QObject::connect(wheelToggle, &ButtonControl::clicked, [functionsMap, longitudinalFunctionsMap, key = param, parent, wheelToggle, this]() mutable {
      if (parent->hasOpenpilotLongitudinal) {
        QMap<int, QString>::const_iterator it;
        for (it = longitudinalFunctionsMap.constBegin(); it != longitudinalFunctionsMap.constEnd(); ++it) {
          functionsMap[it.key()] = it.value();
        }
      }

      QString selection = MultiOptionDialog::getSelection(tr("Select a function to assign to this button"), functionsMap.values(), functionsMap[params.getInt(key.toStdString())], this);
      if (!selection.isEmpty()) {
        params.putInt(key.toStdString(), functionsMap.key(selection));

        wheelToggle->setValue(selection);
      }
    });
    QMap<int, QString> mergedFunctionsMap = functionsMap;
    QMap<int, QString>::const_iterator it;
    for (it = longitudinalFunctionsMap.constBegin(); it != longitudinalFunctionsMap.constEnd(); ++it) {
      mergedFunctionsMap[it.key()] = it.value();
    }
    wheelToggle->setValue(mergedFunctionsMap[params.getInt(param.toStdString())]);

    toggles[param] = wheelToggle;

    addItem(wheelToggle);

    QObject::connect(wheelToggle, &AbstractControl::hideDescriptionEvent, [this]() {
      update();
    });
    QObject::connect(wheelToggle, &AbstractControl::showDescriptionEvent, [this]() {
      update();
    });
  }

  openDescriptions(forceOpenDescriptions, toggles);
}

void FrogPilotWheelPanel::showEvent(QShowEvent *event) {
  frogpilotToggleLevels = parent->frogpilotToggleLevels;

  updateToggles();
}

void FrogPilotWheelPanel::updateToggles() {
  for (auto &[key, toggle] : toggles) {
    bool setVisible = parent->tuningLevel >= frogpilotToggleLevels[key].toDouble();

    if (key == "LKASButtonControl") {
      setVisible &= !parent->isSubaru;
      setVisible &= !(params.getBool("AlwaysOnLateral") && params.getBool("AlwaysOnLateralLKAS"));
    }

    toggle->setVisible(setVisible);
  }

  openDescriptions(forceOpenDescriptions, toggles);

  update();
}
