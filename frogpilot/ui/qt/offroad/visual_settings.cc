#include "frogpilot/ui/qt/offroad/visual_settings.h"

FrogPilotVisualsPanel::FrogPilotVisualsPanel(FrogPilotSettingsWindow *parent) : FrogPilotListWidget(parent), parent(parent) {
  QJsonObject shownDescriptions = QJsonDocument::fromJson(QString::fromStdString(params.get("ShownToggleDescriptions")).toUtf8()).object();
  QString className = this->metaObject()->className();

  if (!shownDescriptions.value(className).toBool(false)) {
    forceOpenDescriptions = true;
    shownDescriptions.insert(className, true);
    params.put("ShownToggleDescriptions", QJsonDocument(shownDescriptions).toJson(QJsonDocument::Compact).toStdString());
  }

  QStackedLayout *visualsLayout = new QStackedLayout();
  addItem(visualsLayout);

  FrogPilotListWidget *visualsList = new FrogPilotListWidget(this);

  ScrollView *visualsPanel = new ScrollView(visualsList, this);

  visualsLayout->addWidget(visualsPanel);

  FrogPilotListWidget *advancedCustomList = new FrogPilotListWidget(this);
  FrogPilotListWidget *customUIList = new FrogPilotListWidget(this);
  FrogPilotListWidget *developerMetricList = new FrogPilotListWidget(this);
  FrogPilotListWidget *developerSidebarList = new FrogPilotListWidget(this);
  FrogPilotListWidget *developerUIList = new FrogPilotListWidget(this);
  FrogPilotListWidget *developerWidgetList = new FrogPilotListWidget(this);
  FrogPilotListWidget *modelUIList = new FrogPilotListWidget(this);
  FrogPilotListWidget *navigationUIList = new FrogPilotListWidget(this);
  FrogPilotListWidget *qualityOfLifeList = new FrogPilotListWidget(this);

  ScrollView *advancedCustomPanel = new ScrollView(advancedCustomList, this);
  ScrollView *customUIPanel = new ScrollView(customUIList, this);
  ScrollView *developerMetricPanel = new ScrollView(developerMetricList, this);
  ScrollView *developerSidebarPanel = new ScrollView(developerSidebarList, this);
  ScrollView *developerUIPanel = new ScrollView(developerUIList, this);
  ScrollView *developerWidgetPanel = new ScrollView(developerWidgetList, this);
  ScrollView *modelUIPanel = new ScrollView(modelUIList, this);
  ScrollView *navigationUIPanel = new ScrollView(navigationUIList, this);
  ScrollView *qualityOfLifePanel = new ScrollView(qualityOfLifeList, this);

  visualsLayout->addWidget(advancedCustomPanel);
  visualsLayout->addWidget(customUIPanel);
  visualsLayout->addWidget(developerMetricPanel);
  visualsLayout->addWidget(developerSidebarPanel);
  visualsLayout->addWidget(developerUIPanel);
  visualsLayout->addWidget(developerWidgetPanel);
  visualsLayout->addWidget(modelUIPanel);
  visualsLayout->addWidget(navigationUIPanel);
  visualsLayout->addWidget(qualityOfLifePanel);

  const std::vector<std::tuple<QString, QString, QString, QString>> visualToggles {
    {"AdvancedCustomUI", tr("Driving Screen Display"), tr("<b>Hide on-screen items and choose which speed readout the driving screen shows.</b>"), "../../frogpilot/assets/toggle_icons/icon_advanced_device.png"},
    {"HideSpeed", tr("Hide Current Speed"), tr("<b>Removes the current speed readout from the driving screen.</b> Turn this on for a cleaner, less cluttered display. Leave it off if you like glancing at your speed on screen."), ""},
    {"HideLeadMarker", tr("Hide Lead Marker"), tr("<b>Hides the marker openpilot draws over the lead vehicle ahead on the driving screen.</b> Turn this on for a cleaner view. Leave it off if you like the marker confirming openpilot is tracking the car you're following."), ""},
    {"HideMapIcon", tr("Hide Map Settings Button"), tr("<b>Hides the map settings button from the driving screen.</b> Turn this on if you do not use the on-screen map shortcut. Leave it off for quick access to map settings.<br><br>- \"Hide Map\": Turn this on when you do not want the map view to appear on the driving screen."), ""},
    {"HideMaxSpeed", tr("Hide Max Speed"), tr("<b>Hides the cruise set speed (\"MAX\") readout from the driving screen.</b> Turn this on to declutter the display. Leave it off to keep the set speed visible at a glance."), ""},
    {"HideAlerts", tr("Hide Non-Critical Alerts"), tr("<b>Hides low-priority status alerts from the driving screen.</b> Turn this on to quiet routine status alerts. Leave it off to keep them visible, since critical and safety alerts still appear either way."), ""},
    {"HideSpeedLimit", tr("Hide Speed Limits"), tr("<b>Hide the posted speed limit sign from the driving screen.</b> Turn this on for a cleaner display. The sign still flashes briefly whenever the limit changes, so leave it off if you want it visible at all times."), ""},
    {"WheelSpeed", tr("Use Wheel Speed"), tr("<b>Shows the vehicle's wheel speed on the driving screen instead of the dashboard cluster speed.</b> Turn this on if you prefer the wheel-speed reading. Leave it off for the dashboard cluster speed, since it does not change how openpilot drives."), ""},

    {"DeveloperUI", tr("Developer UI"), tr("<b>On-screen metrics, overlays, and debug readouts that show what openpilot is doing under the hood.</b>"), "../assets/offroad/icon_shell.png"},
    {"AdjacentPathMetrics", tr("Adjacent Lane Width"), tr("<b>Labels each detected lane beside you with its measured width on the driving screen, or shows \"Vehicle in blind spot\" when \"Blind Spot Path\" is on and a car is there.</b> Turn it on only if you're checking how openpilot reads the neighboring lanes.<br><br>Default: Off."), ""},
    {"DeveloperMetrics", tr("Developer Metrics"), tr("<b>On-screen metrics and system readouts for the driving screen and sidebar.</b>"), ""},
    {"BorderMetrics", tr("Border Metrics"), tr("<b>Adds status cues to the driving screen's border using the chips you enable.</b> These are on-screen only and never change how the car drives, so most drivers can leave them off.<br><br>- \"Blind Spot\": Border turns red when a vehicle is in a blind spot.<br>- \"Steering Torque\": Border shifts green to red with how hard openpilot is steering.<br>- \"Turn Signal\": Border flashes yellow while a turn signal is on.<br><br>Default: Off, with Blind Spot selected when enabled."), ""},
    {"LeadInfo", tr("Lead Vehicle Info"), tr("<b>Shows each lead's distance and speed under its marker.</b> Turn it on to watch tracking details, including the desired following distance and time gap for the lead directly ahead. Leave it off unless you're checking lead-vehicle tracking.<br><br>Default: on in \"Developer Metrics\"."), ""},
    {"FPSCounter", tr("Frame Rate (FPS)"), tr("<b>Shows the live frame rate with min, max, and average at the bottom of the driving screen.</b> Turn it on to spot screen stutter or lag while testing. It's a diagnostic readout, so leave it off unless you're chasing UI performance.<br><br>Default: on in \"Developer Metrics\"."), ""},
    {"NumericalTemp", tr("Numerical Temperature"), tr("<b>Shows the sidebar temperature as a number instead of a status word (GOOD / OK / HIGH).</b> The word readout is fine for almost everyone, so this is a developer extra.<br><br>- \"Fahrenheit\": Shows the number in degrees F instead of degrees C.<br><br>Default: number in degrees C."), ""},
    {"SidebarMetrics", tr("Sidebar Metrics"), tr("<b>Pick which system stats show in the sidebar while driving.</b> The default pair is plenty for most drivers.<br><br>- \"CPU\" / \"GPU\": Processor load %<br>- \"IP\": Device IP address (replaces the Wi-Fi bars)<br>- \"RAM\": Memory usage %<br>- \"SSD Left\" / \"SSD Used\": Free or used storage, in GB<br><br>Default: CPU and RAM on."), ""},
    {"UseSI", tr("International System of Units"), tr("<b>Shows FrogPilot's developer metric readouts in International System of Units (SI) units like meters and m/s instead of your normal display units.</b> Turn it on only if you read sensor data in SI. Leave it off to match the units used everywhere else in openpilot.<br><br>Default: on."), ""},
    {"DeveloperSidebar", tr("Developer Sidebar"), tr("<b>Shows a right-side driving-screen sidebar with up to 7 live tuning and engagement readouts you choose.</b>"), ""},
    {"DeveloperSidebarMetric1", tr("Metric #1"), tr("<b>Select the metric shown in the first \"Developer Sidebar\" widget.</b>"), ""},
    {"DeveloperSidebarMetric2", tr("Metric #2"), tr("<b>Select the metric shown in the second \"Developer Sidebar\" widget.</b>"), ""},
    {"DeveloperSidebarMetric3", tr("Metric #3"), tr("<b>Select the metric shown in the third \"Developer Sidebar\" widget.</b>"), ""},
    {"DeveloperSidebarMetric4", tr("Metric #4"), tr("<b>Select the metric shown in the fourth \"Developer Sidebar\" widget.</b>"), ""},
    {"DeveloperSidebarMetric5", tr("Metric #5"), tr("<b>Select the metric shown in the fifth \"Developer Sidebar\" widget.</b>"), ""},
    {"DeveloperSidebarMetric6", tr("Metric #6"), tr("<b>Select the metric shown in the sixth \"Developer Sidebar\" widget.</b>"), ""},
    {"DeveloperSidebarMetric7", tr("Metric #7"), tr("<b>Select the metric shown in the seventh \"Developer Sidebar\" widget.</b>"), ""},
    {"DeveloperWidgets", tr("Developer Widgets"), tr("<b>Debugging overlays for radar tracking and the model's predictions on the driving screen.</b>"), ""},
    {"AdjacentLeadsUI", tr("Adjacent Lead Markers"), tr("<b>Marks radar-detected vehicles in the lanes to your left and right, alongside the usual lead-ahead marker.</b> Turn it on for adjacent-lead debugging. Turn it off to reduce clutter, since this is a developer readout most drivers don't need.<br><br>Default: on."), ""},
    {"ShowStoppingPoint", tr("Stopping Point Marker"), tr("<b>Shows a stop-sign marker on the road where openpilot plans to stop.</b> Leave it on to preview that stopping point. Turn it off if the marker is distracting.<br><br>- \"Show Distance\": also shows how far away that stopping point is.<br><br>Default: marker on, distance on."), ""},
    {"RadarTracksUI", tr("Radar Tracks"), tr("<b>Draws a red dot on the driving screen for every raw radar return the car reports.</b> Turn it on only when debugging radar input. It clutters the road view and changes nothing about how the car drives, so it stays off for most drivers.<br><br>Default: off."), ""},

    {"CustomUI", tr("Driving Screen Widgets"), tr("<b>Optional widgets and overlays for the driving screen, like a compass, lane and path overlays, and on-screen pedal indicators.</b>"), "../assets/offroad/icon_road.png"},
    {"AccelerationPath", tr("Color Path by Acceleration"), tr("<b>Tints the on-screen driving path by what openpilot plans to do: green where it speeds up, red where it brakes.</b> Turn it on for an at-a-glance read of upcoming acceleration and braking. Leave it off to keep the normal path color unless \"Experimental Mode\" is on, which colors the path this way regardless."), ""},
    {"AdjacentPath", tr("Adjacent Lane Paths"), tr("<b>Shades the lanes next to you from green (roomy) to red (tight) on the driving screen, turning solid red when \"Blind Spot Path\" is on and a car sits in that blind spot.</b> Switch it on to watch how openpilot reads the lanes beside you, or off for a cleaner screen.<br><br>Default: Off."), ""},
    {"BlindSpotPath", tr("Blind Spot Path"), tr("<b>Shade the next lane over in red whenever your car's blind spot monitor detects a vehicle there.</b> Turn this on for an extra on-screen blind-spot warning. Leave it off if you'd rather keep the driving screen clean."), ""},
    {"Compass", tr("Compass"), tr("<b>Show your current driving direction with a simple on-screen compass.</b> Turn it on if you like seeing which way you're heading at a glance. Leave it off for a cleaner driving screen."), ""},
    {"OnroadDistanceButton", tr("On-Screen Personality Button"), tr("<b>Adds a button to the driving screen that shows the current driving personality and switches it with a tap.</b> Turn it on to change personality from the road without opening a menu. The profile sets both your following distance and how assertively openpilot uses the gas and brake."), ""},
    {"PedalsOnUI", tr("Gas / Brake Pedal Icons"), tr("<b>Shows gas and brake pedal icons on the driving screen.</b> Turn it on to watch when openpilot is using the gas or the brake. Leave it off for a cleaner screen.<br><br>- \"Dynamic\": icon opacity tracks roughly how hard openpilot is accelerating or braking.<br>- \"Static\": each icon is full while that pedal is active and dim otherwise."), ""},
    {"RotatingWheel", tr("Rotating Steering Wheel"), tr("<b>The on-screen steering-wheel icon spins to match your real steering wheel's angle.</b> Turn it on for an at-a-glance read of how much the steering wheel is turned. Leave it off for a steadier, less busy icon."), ""},

    {"ModelUI", tr("Path & Lines Visuals"), tr("<b>How the driving path, lane lines, and road edges are drawn on the driving screen.</b>"), "../../frogpilot/assets/toggle_icons/icon_road.png"},
    {"DynamicPathWidth", tr("Dynamic Path Width"), tr("<b>Narrows the on-screen path as openpilot's engagement drops, so its width shows the current state at a glance.</b> Turn this on if you want the path to double as an engagement cue. Leave it off to keep the path full width always.<br><br>\"Fully Engaged\": 100%<br>\"Always On Lateral\": 75%<br>\"Disengaged\": 50%"), ""},
    {"LaneLinesWidth", tr("Lane Line Width"), tr("<b>Set how thick the lane lines are drawn on the driving screen.</b> Raise it to make the lane lines easier to see, lower it for a cleaner view, or set it to 0 to hide them.<br><br>Default: 4 inches."), ""},
    {"PathEdgeWidth", tr("Path Edge Width"), tr("<b>Sets how wide the colored edges along the driving path are drawn, as a percent of the path width.</b> Raise it to make openpilot's mode color easier to spot at a glance. Lower it for a thinner accent, or set it to \"Off\" for no edges.<br><br>Color Guide:<br><br>- \"Blue\": Navigation<br>- \"Light Blue\": \"Always On Lateral\"<br>- \"Green\": Default<br>- \"Orange\": \"Experimental Mode\"<br>- \"Red\": \"Traffic Mode\"<br>- \"Yellow\": Conditional Experimental Mode overridden<br><br>Default: 20%."), ""},
    {"PathWidth", tr("Path Width"), tr("<b>Sets how wide the driving-path overlay is drawn on-screen.</b> Raise it for a bolder path, lower it for a thinner one, or set it to \"Off\" to hide the overlay entirely.<br><br>Default: 6.1 feet."), ""},
    {"RoadEdgesWidth", tr("Road Edge Width"), tr("<b>Sets how thick the road-edge lines are drawn on the driving display.</b> Raise it to make the road edges easier to see at a glance. Lower it for a cleaner look, or set it to \"Off\" to hide them.<br><br>Default: 2 inches."), ""},
    {"UnlimitedLength", tr("Full-Length Road UI"), tr("<b>Draw the path, lane lines, and road edges as far ahead as openpilot can see.</b> Leave on for the longest view. Turn off if you prefer a shorter, less cluttered overlay on the driving screen.<br><br>Default: on."), ""},

    {"NavigationUI", tr("Navigation Widgets"), tr("<b>Map display, road name, and speed-limit signs shown while driving.</b>"), "../../frogpilot/assets/toggle_icons/icon_map.png"},
    {"BigMap", tr("Larger Map"), tr("<b>Widens the on-screen map to three-quarters of the screen, with the road camera still beside it.</b> Turn this on for easier map reading. Leave \"Full Map\" off unless you can drive without on-screen alerts.<br><br>- \"Full Map\": Makes the map fill the whole screen.<br><br><i><b>Disclaimer</b>: With \"Full Map\" on, the camera feed and openpilot's alerts are hidden while the map is open, so you can miss an alert.</i>"), ""},
    {"MapStyle", tr("Map Style"), tr("<b>Sets how the map looks during \"Navigate on openpilot\" (NOO).</b> Pick the style you find easiest to read while driving:<br><br>- \"Stock openpilot\": Default comma.ai style<br>- \"FrogPilot\": Official FrogPilot map style<br>- \"Mapbox Streets\": Standard street-focused view<br>- \"Mapbox Outdoors\": Emphasizes outdoor and terrain features<br>- \"Mapbox Light\": Minimalist, bright theme<br>- \"Mapbox Dark\": Minimalist, dark theme<br>- \"Mapbox Navigation Day\": Optimized for daytime navigation<br>- \"Mapbox Navigation Night\": Optimized for nighttime navigation<br>- \"Mapbox Satellite\": Satellite imagery only<br>- \"Mapbox Satellite Streets\": Satellite imagery with street labels<br>- \"Mapbox Traffic Night\": Dark theme emphasizing traffic conditions<br>- \"Mike's Personalized Style\": Customized hybrid satellite view"), ""},
    {"RoadNameUI", tr("Road Name"), tr("<b>Shows the name of the road you're on at the bottom of the driving screen</b>, using data from \"OpenStreetMap (OSM)\". Turn it on if you like seeing where you are at a glance. Leave it off for a cleaner screen."), ""},
    {"ShowSpeedLimits", tr("Show Speed Limits"), tr("<b>Shows the posted speed-limit sign in the top-left corner of the driving screen.</b> Turn it on to keep an eye on the limit at a glance. Leave it off for a cleaner screen, or if you already use \"Speed Limit Controller\"."), ""},
    {"SLCMapboxFiller", tr("Fill Speed Limits with Mapbox"), tr("<b>Fill the on-screen speed-limit sign with Mapbox data when the dashboard and \"OpenStreetMap (OSM)\" have none.</b> Turn this on if \"Show Speed Limits\" often leaves the sign blank in your area. Leave it off if another source usually has it."), ""},
    {"UseVienna", tr("European Speed-Limit Signs"), tr("<b>Draws the on-screen speed-limit sign in the European (round) style instead of the US (rectangular) style.</b> Turn it on if you drive where round European signs are used. Leave it off for US-style signs, since this only changes how the sign looks, not the limit it shows."), ""},

    {"QOLVisuals", tr("Quality of Life"), tr("<b>Driving-screen conveniences, including the camera view, the cabin camera while reversing, and a stopped timer.</b>"), "../../frogpilot/assets/toggle_icons/icon_quality_of_life.png"},
    {"CameraView", tr("Camera View"), tr("<b>Choose which camera feed shows on the driving screen.</b> This is purely visual and doesn't change how openpilot drives.<br><br>- \"Auto\": Uses the standard road camera, switching to wide at low speed in \"Experimental Mode\" on cars with a wide camera<br>- \"Driver\": The cabin-facing camera that watches the driver<br>- \"Standard\": The narrow forward road camera<br>- \"Wide\": The wide-angle forward road camera"), ""},
    {"DriverCamera", tr("Driver Camera In Reverse"), tr("<b>Switches the screen to the cabin-facing driver camera when the car is in reverse.</b> Turn this on if you like seeing the cabin view while backing up. Leave it off otherwise, since this camera faces inward and is not a rearview of what's behind the car.<br><br>Default: Off."), ""},
    {"StoppedTimer", tr("Stopped Timer"), tr("<b>When stopped, replaces the centered speed readout with a count-up timer showing how long you've been at a standstill.</b> Turn this on if you like tracking long waits at lights or in traffic. Leave it off to keep the normal speed display."), ""}
  };

  for (const auto &[param, title, desc, icon] : visualToggles) {
    AbstractControl *visualToggle;

    if (param == "AdvancedCustomUI") {
      FrogPilotManageControl *advancedCustomUIToggle = new FrogPilotManageControl(param, title, desc, icon);
      QObject::connect(advancedCustomUIToggle, &FrogPilotManageControl::manageButtonClicked, [visualsLayout, advancedCustomPanel]() {
        visualsLayout->setCurrentWidget(advancedCustomPanel);
      });
      visualToggle = advancedCustomUIToggle;
    } else if (param == "HideMapIcon") {
      std::vector<QString> mapIconToggles{"HideMap"};
      std::vector<QString> mapIconToggleNames{tr("Hide Map")};
      visualToggle = new FrogPilotButtonToggleControl(param, title, desc, icon, mapIconToggles, mapIconToggleNames);

    } else if (param == "DeveloperUI") {
      FrogPilotManageControl *developerUIToggle = new FrogPilotManageControl(param, title, desc, icon);
      QObject::connect(developerUIToggle, &FrogPilotManageControl::manageButtonClicked, [visualsLayout, developerUIPanel]() {
        visualsLayout->setCurrentWidget(developerUIPanel);
      });
      visualToggle = developerUIToggle;
    } else if (param == "DeveloperMetrics") {
      FrogPilotManageControl *developerMetricsToggle = new FrogPilotManageControl(param, title, desc, icon);
      QObject::connect(developerMetricsToggle, &FrogPilotManageControl::manageButtonClicked, [visualsLayout, developerMetricPanel, this]() {
        openSubSubPanel();

        visualsLayout->setCurrentWidget(developerMetricPanel);

        developerUIOpen = true;
      });
      visualToggle = developerMetricsToggle;
    } else if (param == "BorderMetrics") {
      std::vector<QString> borderToggles{"BlindSpotMetrics", "ShowSteering", "SignalMetrics"};
      std::vector<QString> borderToggleNames{tr("Blind Spot"), tr("Steering Torque"), tr("Turn Signal")};
      borderMetricsButton = new FrogPilotButtonToggleControl(param, title, desc, icon, borderToggles, borderToggleNames);
      visualToggle = borderMetricsButton;
    } else if (param == "NumericalTemp") {
      std::vector<QString> temperatureToggles{"Fahrenheit"};
      std::vector<QString> temperatureToggleNames{tr("Fahrenheit")};
      visualToggle = new FrogPilotButtonToggleControl(param, title, desc, icon, temperatureToggles, temperatureToggleNames);
    } else if (param == "SidebarMetrics") {
      sidebarMetricsToggles = {"ShowCPU", "ShowGPU", "ShowIP", "ShowMemoryUsage", "ShowStorageLeft", "ShowStorageUsed"};
      std::vector<QString> sidebarMetricsToggleNames{tr("CPU"), tr("GPU"), tr("IP"), tr("RAM"), tr("SSD Left"), tr("SSD Used")};
      sidebarMetricsToggle = new FrogPilotButtonsControl(title, desc, icon, sidebarMetricsToggleNames, true, false, 150);
      for (int i = 0; i < sidebarMetricsToggles.size(); ++i) {
        if (params.getBool(sidebarMetricsToggles[i].toStdString())) {
          sidebarMetricsToggle->setCheckedButton(i);
        }
      }
      QObject::connect(sidebarMetricsToggle, &FrogPilotButtonsControl::buttonClicked, [this](int id) {
        params.putBool(sidebarMetricsToggles[id].toStdString(), !params.getBool(sidebarMetricsToggles[id].toStdString()));

        if (id == 0) {
          params.putBool("ShowGPU", false);
        } else if (id == 1) {
          params.putBool("ShowCPU", false);
        } else if (id == 3) {
          params.putBool("ShowStorageLeft", false);
          params.putBool("ShowStorageUsed", false);
        } else if (id == 4) {
          params.putBool("ShowMemoryUsage", false);
          params.putBool("ShowStorageUsed", false);
        } else if (id == 5) {
          params.putBool("ShowMemoryUsage", false);
          params.putBool("ShowStorageLeft", false);
        }

        sidebarMetricsToggle->clearCheckedButtons();
        for (int i = 0; i < sidebarMetricsToggles.size(); ++i) {
          if (params.getBool(sidebarMetricsToggles[i].toStdString())) {
            sidebarMetricsToggle->setCheckedButton(i);
          }
        }
      });
      visualToggle = sidebarMetricsToggle;
    } else if (param == "DeveloperSidebar") {
      FrogPilotManageControl *developerSidebarToggle = new FrogPilotManageControl(param, title, desc, icon);
      QObject::connect(developerSidebarToggle, &FrogPilotManageControl::manageButtonClicked, [visualsLayout, developerSidebarPanel, this]() {
        openSubSubPanel();

        visualsLayout->setCurrentWidget(developerSidebarPanel);

        developerUIOpen = true;
      });
      visualToggle = developerSidebarToggle;
    } else if (developerSidebarKeys.contains(param)) {
      QMap<int, QString> developerSidebarMetricOptions {
        {0, tr("None")},
        {1, tr("Acceleration: Current")},
        {2, tr("Acceleration: Max")},
        {3, tr("Auto Tune: Actuator Delay")},
        {4, tr("Auto Tune: Friction")},
        {5, tr("Auto Tune: Lateral Acceleration")},
        {6, tr("Auto Tune: Steer Ratio")},
        {7, tr("Auto Tune: Stiffness Factor")},
        {8, tr("Engagement %: Lateral")},
        {9, tr("Engagement %: Longitudinal")},
        {10, tr("Lateral Control: Steering Angle")},
        {11, tr("Lateral Control: Torque % Used")},
        {12, tr("Longitudinal Control: Actuator Acceleration Output")},
        {13, tr("Longitudinal MPC: Danger Factor")},
        {14, tr("Longitudinal MPC Jerk: Acceleration")},
        {15, tr("Longitudinal MPC Jerk: Danger Zone")},
        {16, tr("Longitudinal MPC Jerk: Speed Control")},
      };

      ButtonControl *metricToggle = new ButtonControl(title, tr("SELECT"), desc);
      QObject::connect(metricToggle, &ButtonControl::clicked, [metricToggle, key = param, developerSidebarMetricOptions, this]() mutable {
        QString current = developerSidebarMetricOptions.value(params.getInt(key.toStdString()), tr("None"));
        QString selection = MultiOptionDialog::getSelection(tr("Select a metric to display"), developerSidebarMetricOptions.values(), current, this);

        if (!selection.isEmpty()) {
          int selectedMetric = developerSidebarMetricOptions.key(selection);

          params.putInt(key.toStdString(), selectedMetric);

          metricToggle->setValue(selection);
        }
      });
      metricToggle->setValue(developerSidebarMetricOptions.value(params.getInt(param.toStdString()), tr("None")));
      visualToggle = metricToggle;
    } else if (param == "DeveloperWidgets") {
      FrogPilotManageControl *developerWidgetsToggle = new FrogPilotManageControl(param, title, desc, icon);
      QObject::connect(developerWidgetsToggle, &FrogPilotManageControl::manageButtonClicked, [visualsLayout, developerWidgetPanel, this]() {
        openSubSubPanel();

        visualsLayout->setCurrentWidget(developerWidgetPanel);

        developerUIOpen = true;
      });
      visualToggle = developerWidgetsToggle;
    } else if (param == "ShowStoppingPoint") {
      std::vector<QString> stoppingPointToggles{"ShowStoppingPointMetrics"};
      std::vector<QString> stoppingPointToggleNames{tr("Show Distance")};
      visualToggle = new FrogPilotButtonToggleControl(param, title, desc, icon, stoppingPointToggles, stoppingPointToggleNames);

    } else if (param == "CustomUI") {
      FrogPilotManageControl *customUIToggle = new FrogPilotManageControl(param, title, desc, icon);
      QObject::connect(customUIToggle, &FrogPilotManageControl::manageButtonClicked, [visualsLayout, customUIPanel]() {
        visualsLayout->setCurrentWidget(customUIPanel);
      });
      visualToggle = customUIToggle;
    } else if (param == "PedalsOnUI") {
      std::vector<QString> pedalsToggles{"DynamicPedalsOnUI", "StaticPedalsOnUI"};
      std::vector<QString> pedalsToggleNames{tr("Dynamic"), tr("Static")};
      FrogPilotButtonToggleControl *pedalsToggle = new FrogPilotButtonToggleControl(param, title, desc, icon, pedalsToggles, pedalsToggleNames, true);
      QObject::connect(pedalsToggle, &FrogPilotButtonToggleControl::buttonClicked, [this](int id) {
        if (id == 0) {
          params.putBool("StaticPedalsOnUI", false);
        } else if (id == 1) {
          params.putBool("DynamicPedalsOnUI", false);
        }
      });
      visualToggle = pedalsToggle;

    } else if (param == "ModelUI") {
      FrogPilotManageControl *modelUIToggle = new FrogPilotManageControl(param, title, desc, icon);
      QObject::connect(modelUIToggle, &FrogPilotManageControl::manageButtonClicked, [visualsLayout, modelUIPanel]() {
        visualsLayout->setCurrentWidget(modelUIPanel);
      });
      visualToggle = modelUIToggle;
    } else if (param == "LaneLinesWidth" || param == "RoadEdgesWidth") {
      visualToggle = new FrogPilotParamValueControl(param, title, desc, icon, 0, 24, tr(" inches"));
    } else if (param == "PathEdgeWidth") {
      std::map<float, QString> pathEdgeLabels;
      for (int i = 0; i <= 100; ++i) {
        pathEdgeLabels[i] = i == 0 ? tr("Off") : QString::number(i) + "%";
      }
      visualToggle = new FrogPilotParamValueControl(param, title, desc, icon, 0, 100, QString(), pathEdgeLabels);
    } else if (param == "PathWidth") {
      visualToggle = new FrogPilotParamValueControl(param, title, desc, icon, 0, 10, tr(" feet"), std::map<float, QString>(), 0.1);

    } else if (param == "NavigationUI") {
      FrogPilotManageControl *navigationUIToggle = new FrogPilotManageControl(param, title, desc, icon);
      QObject::connect(navigationUIToggle, &FrogPilotManageControl::manageButtonClicked, [visualsLayout, navigationUIPanel]() {
        visualsLayout->setCurrentWidget(navigationUIPanel);
      });
      visualToggle = navigationUIToggle;
    } else if (param == "BigMap") {
      std::vector<QString> mapToggles{"FullMap"};
      std::vector<QString> mapToggleNames{tr("Full Map")};
      visualToggle = new FrogPilotButtonToggleControl(param, title, desc, icon, mapToggles, mapToggleNames);
    } else if (param == "MapStyle") {
      QMap<int, QString> styleMap {
        {0, tr("Stock openpilot")},
        {1, tr("FrogPilot")},
        {2, tr("Mapbox Streets")},
        {3, tr("Mapbox Outdoors")},
        {4, tr("Mapbox Light")},
        {5, tr("Mapbox Dark")},
        {6, tr("Mapbox Navigation Day")},
        {7, tr("Mapbox Navigation Night")},
        {8, tr("Mapbox Satellite")},
        {9, tr("Mapbox Satellite Streets")},
        {10, tr("Mapbox Traffic Night")},
        {11, tr("Mike's Personalized Style")}
      };

      ButtonControl *mapStyleButton = new ButtonControl(title, tr("SELECT"), desc);
      QObject::connect(mapStyleButton, &ButtonControl::clicked, [mapStyleButton, styleMap, this]() {
        QString selection = MultiOptionDialog::getSelection(tr("Select a map style"), styleMap.values(), "", this);
        if (!selection.isEmpty()) {
          int selectedStyle = styleMap.key(selection);

          params.putInt("MapStyle", selectedStyle);

          mapStyleButton->setValue(selection);
        }
      });
      int currentStyle = params.getInt("MapStyle");
      mapStyleButton->setValue(styleMap[currentStyle]);

      visualToggle = mapStyleButton;

    } else if (param == "QOLVisuals") {
      FrogPilotManageControl *qolToggle = new FrogPilotManageControl(param, title, desc, icon);
      QObject::connect(qolToggle, &FrogPilotManageControl::manageButtonClicked, [visualsLayout, qualityOfLifePanel]() {
        visualsLayout->setCurrentWidget(qualityOfLifePanel);
      });
      visualToggle = qolToggle;
    } else if (param == "CameraView") {
      std::vector<QString> cameraOptions{tr("Auto"), tr("Driver"), tr("Standard"), tr("Wide")};
      ButtonParamControl *cameraSelection = new ButtonParamControl(param, title, desc, icon, cameraOptions);
      visualToggle = cameraSelection;

    } else {
      visualToggle = new ParamControl(param, title, desc, icon);
    }

    toggles[param] = visualToggle;

    if (advancedCustomOnroadUIKeys.contains(param)) {
      advancedCustomList->addItem(visualToggle);
    } else if (customOnroadUIKeys.contains(param)) {
      customUIList->addItem(visualToggle);
    } else if (developerMetricKeys.contains(param)) {
      developerMetricList->addItem(visualToggle);
    } else if (developerSidebarKeys.contains(param)) {
      developerSidebarList->addItem(visualToggle);
    } else if (developerUIKeys.contains(param)) {
      developerUIList->addItem(visualToggle);
    } else if (developerWidgetKeys.contains(param)) {
      developerWidgetList->addItem(visualToggle);
    } else if (modelUIKeys.contains(param)) {
      modelUIList->addItem(visualToggle);
    } else if (navigationUIKeys.contains(param)) {
      navigationUIList->addItem(visualToggle);
    } else if (qualityOfLifeKeys.contains(param)) {
      qualityOfLifeList->addItem(visualToggle);
    } else {
      visualsList->addItem(visualToggle);

      parentKeys.insert(param);
    }

    if (FrogPilotManageControl *frogPilotManageToggle = qobject_cast<FrogPilotManageControl*>(visualToggle)) {
      QObject::connect(frogPilotManageToggle, &FrogPilotManageControl::manageButtonClicked, [this]() {
        emit openSubPanel();
        openDescriptions(forceOpenDescriptions, toggles);
      });
    }

    QObject::connect(visualToggle, &AbstractControl::hideDescriptionEvent, [this]() {
      update();
    });
    QObject::connect(visualToggle, &AbstractControl::showDescriptionEvent, [this]() {
      update();
    });
  }

  QSet<QString> forceUpdateKeys = {"HideLeadMarker", "ShowSpeedLimits"};
  for (const QString &key : forceUpdateKeys) {
    QObject::connect(static_cast<ToggleControl*>(toggles[key]), &ToggleControl::toggleFlipped, this, &FrogPilotVisualsPanel::updateToggles);
  }

  openDescriptions(forceOpenDescriptions, toggles);

  QObject::connect(parent, &FrogPilotSettingsWindow::closeSubPanel, [visualsLayout, visualsPanel, this] {
    openDescriptions(forceOpenDescriptions, toggles);
    visualsLayout->setCurrentWidget(visualsPanel);
  });
  QObject::connect(parent, &FrogPilotSettingsWindow::closeSubSubPanel, [visualsLayout, developerUIPanel, this]() {
    openDescriptions(forceOpenDescriptions, toggles);

    if (developerUIOpen) {
      visualsLayout->setCurrentWidget(developerUIPanel);

      developerUIOpen = false;
    }
  });
  QObject::connect(parent, &FrogPilotSettingsWindow::updateMetric, this, &FrogPilotVisualsPanel::updateMetric);
}

void FrogPilotVisualsPanel::showEvent(QShowEvent *event) {
  frogpilotToggleLevels = parent->frogpilotToggleLevels;

  for (int i = 0; i < sidebarMetricsToggles.size(); ++i) {
    if (params.getBool(sidebarMetricsToggles[i].toStdString())) {
      sidebarMetricsToggle->setCheckedButton(i);
    }
  }

  updateToggles();
}

void FrogPilotVisualsPanel::updateMetric(bool metric, bool bootRun) {
  static bool previousMetric;
  if (metric != previousMetric && !bootRun) {
    double distanceConversion = metric ? FOOT_TO_METER : METER_TO_FOOT;
    double smallDistanceConversion = metric ? INCH_TO_CM : CM_TO_INCH;

    params.putIntNonBlocking("LaneLinesWidth", params.getInt("LaneLinesWidth") * smallDistanceConversion);
    params.putIntNonBlocking("RoadEdgesWidth", params.getInt("RoadEdgesWidth") * smallDistanceConversion);

    params.putFloatNonBlocking("PathWidth", params.getFloat("PathWidth") * distanceConversion);
  }
  previousMetric = metric;

  static std::map<float, QString> imperialDistanceLabels;
  static std::map<float, QString> imperialSmallDistanceLabels;
  static std::map<float, QString> metricDistanceLabels;
  static std::map<float, QString> metricSmallDistanceLabels;

  static bool labelsInitialized = false;
  if (!labelsInitialized) {
    for (int i = 0; i <= 10; ++i) {
      imperialDistanceLabels[i] = i == 0 ? tr("Off") : i == 1 ? QString::number(i) + tr(" foot") : QString::number(i) + tr(" feet");
    }

    for (int i = 0; i <= 24; ++i) {
      imperialSmallDistanceLabels[i] = i == 0 ? tr("Off") : i == 1 ? QString::number(i) + tr(" inch") : QString::number(i) + tr(" inches");
    }

    for (float i = 0.0f; i <= 3.0f; i += 0.1f) {
      metricDistanceLabels[i] = i == 0.0f ? tr("Off") : i == 1.0 ? QString::number(i) + tr(" meter") : QString::number(i, 'f', 1) + tr(" meters");
    }

    for (int i = 0; i <= 60; ++i) {
      metricSmallDistanceLabels[i] = i == 0 ? tr("Off") : i == 1 ? QString::number(i) + tr(" centimeter") : QString::number(i) + tr(" centimeters");
    }

    labelsInitialized = true;
  }

  FrogPilotParamValueControl *laneLinesWidthToggle = static_cast<FrogPilotParamValueControl*>(toggles["LaneLinesWidth"]);
  FrogPilotParamValueControl *pathWidthToggle = static_cast<FrogPilotParamValueControl*>(toggles["PathWidth"]);
  FrogPilotParamValueControl *roadEdgesWidthToggle = static_cast<FrogPilotParamValueControl*>(toggles["RoadEdgesWidth"]);

  if (metric) {
    laneLinesWidthToggle->setDescription(tr("<b>Set how thick the lane lines are drawn on the driving screen.</b> Raise it to make the lane lines easier to see, lower it for a cleaner view, or set it to 0 to hide them.<br><br>Default: 10 centimeters."));
    pathWidthToggle->setDescription(tr("<b>Sets how wide the driving-path overlay is drawn on-screen.</b> Raise it for a bolder path, lower it for a thinner one, or set it to \"Off\" to hide the overlay entirely.<br><br>Default: 1.9 meters."));
    roadEdgesWidthToggle->setDescription(tr("<b>Sets how thick the road-edge lines are drawn on the driving display.</b> Raise it to make the road edges easier to see at a glance. Lower it for a cleaner look, or set it to \"Off\" to hide them.<br><br>Default: 5 centimeters."));

    laneLinesWidthToggle->updateControl(0, 60, metricSmallDistanceLabels);
    roadEdgesWidthToggle->updateControl(0, 60, metricSmallDistanceLabels);

    pathWidthToggle->updateControl(0, 3, metricDistanceLabels);
  } else {
    laneLinesWidthToggle->setDescription(tr("<b>Set how thick the lane lines are drawn on the driving screen.</b> Raise it to make the lane lines easier to see, lower it for a cleaner view, or set it to 0 to hide them.<br><br>Default: 4 inches."));
    pathWidthToggle->setDescription(tr("<b>Sets how wide the driving-path overlay is drawn on-screen.</b> Raise it for a bolder path, lower it for a thinner one, or set it to \"Off\" to hide the overlay entirely.<br><br>Default: 6.1 feet."));
    roadEdgesWidthToggle->setDescription(tr("<b>Sets how thick the road-edge lines are drawn on the driving display.</b> Raise it to make the road edges easier to see at a glance. Lower it for a cleaner look, or set it to \"Off\" to hide them.<br><br>Default: 2 inches."));

    laneLinesWidthToggle->updateControl(0, 24, imperialSmallDistanceLabels);
    roadEdgesWidthToggle->updateControl(0, 24, imperialSmallDistanceLabels);

    pathWidthToggle->updateControl(0, 10, imperialDistanceLabels);
  }
}

void FrogPilotVisualsPanel::updateToggles() {
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

    if (key == "AccelerationPath") {
      setVisible &= parent->hasOpenpilotLongitudinal;
    }

    else if (key == "AdjacentLeadsUI") {
      setVisible &= parent->hasRadar && !(params.getBool("AdvancedCustomUI") && params.getBool("HideLeadMarker"));
    }

    else if (key == "BlindSpotPath") {
      setVisible &= parent->hasBSM;
    }

    else if (key == "HideLeadMarker") {
      setVisible &= parent->hasOpenpilotLongitudinal;
    }

    else if (key == "LeadInfo") {
      setVisible &= parent->hasOpenpilotLongitudinal;
    }

    else if (key == "OnroadDistanceButton") {
      setVisible &= parent->hasOpenpilotLongitudinal;
    }

    else if (key == "PedalsOnUI") {
      setVisible &= parent->hasOpenpilotLongitudinal;
    }

    else if (key == "RadarTracksUI") {
      setVisible &= parent->hasRadar;
    }

    else if (key == "ShowSpeedLimits") {
      setVisible &= !params.getBool("SpeedLimitController") || !parent->hasOpenpilotLongitudinal;
    }

    else if (key == "ShowStoppingPoint") {
      setVisible &= parent->hasOpenpilotLongitudinal;
    }

    else if (key == "SLCMapboxFiller") {
      setVisible &= params.getBool("ShowSpeedLimits") && !(parent->hasOpenpilotLongitudinal && params.getBool("SpeedLimitController"));
      setVisible &= !params.get("MapboxSecretKey").empty();
    }

    toggle->setVisible(setVisible);

    if (setVisible) {
      if (advancedCustomOnroadUIKeys.contains(key)) {
        toggles["AdvancedCustomUI"]->setVisible(true);
      } else if (customOnroadUIKeys.contains(key)) {
        toggles["CustomUI"]->setVisible(true);
      } else if (developerMetricKeys.contains(key)) {
        toggles["DeveloperMetrics"]->setVisible(true);
      } else if (developerUIKeys.contains(key)) {
        toggles["DeveloperUI"]->setVisible(true);
      } else if (developerWidgetKeys.contains(key)) {
        toggles["DeveloperWidgets"]->setVisible(true);
      } else if (modelUIKeys.contains(key)) {
        toggles["ModelUI"]->setVisible(true);
      } else if (navigationUIKeys.contains(key)) {
        toggles["NavigationUI"]->setVisible(true);
      } else if (qualityOfLifeKeys.contains(key)) {
        toggles["QOLVisuals"]->setVisible(true);
      }
    }
  }

  borderMetricsButton->setVisibleButton(0, parent->hasBSM);

  openDescriptions(forceOpenDescriptions, toggles);

  update();
}
