#include "frogpilot/ui/qt/offroad/longitudinal_settings.h"

FrogPilotLongitudinalPanel::FrogPilotLongitudinalPanel(FrogPilotSettingsWindow *parent) : FrogPilotListWidget(parent), parent(parent) {
  networkManager = new QNetworkAccessManager(this);

  QJsonObject shownDescriptions = QJsonDocument::fromJson(QString::fromStdString(params.get("ShownToggleDescriptions")).toUtf8()).object();
  QString className = this->metaObject()->className();

  if (!shownDescriptions.value(className).toBool(false)) {
    forceOpenDescriptions = true;
    shownDescriptions.insert(className, true);
    params.put("ShownToggleDescriptions", QJsonDocument(shownDescriptions).toJson(QJsonDocument::Compact).toStdString());
  }

  QStackedLayout *longitudinalLayout = new QStackedLayout();
  addItem(longitudinalLayout);

  FrogPilotListWidget *longitudinalList = new FrogPilotListWidget(this);

  ScrollView *longitudinalPanel = new ScrollView(longitudinalList, this);

  longitudinalLayout->addWidget(longitudinalPanel);

  FrogPilotListWidget *advancedLongitudinalTuneList = new FrogPilotListWidget(this);
  FrogPilotListWidget *aggressivePersonalityList = new FrogPilotListWidget(this);
  FrogPilotListWidget *conditionalExperimentalList = new FrogPilotListWidget(this);
  FrogPilotListWidget *curveSpeedList = new FrogPilotListWidget(this);
  FrogPilotListWidget *customDrivingPersonalityList = new FrogPilotListWidget(this);
  FrogPilotListWidget *longitudinalTuneList = new FrogPilotListWidget(this);
  FrogPilotListWidget *qolList = new FrogPilotListWidget(this);
  FrogPilotListWidget *relaxedPersonalityList = new FrogPilotListWidget(this);
  FrogPilotListWidget *speedLimitControllerList = new FrogPilotListWidget(this);
  FrogPilotListWidget *speedLimitControllerOffsetsList = new FrogPilotListWidget(this);
  FrogPilotListWidget *speedLimitControllerQOLList = new FrogPilotListWidget(this);
  FrogPilotListWidget *speedLimitControllerVisualList = new FrogPilotListWidget(this);
  FrogPilotListWidget *standardPersonalityList = new FrogPilotListWidget(this);
  FrogPilotListWidget *weatherList = new FrogPilotListWidget(this);
  FrogPilotListWidget *weatherLowVisibilityList = new FrogPilotListWidget(this);
  FrogPilotListWidget *weatherRainList = new FrogPilotListWidget(this);
  FrogPilotListWidget *weatherRainStormList = new FrogPilotListWidget(this);
  FrogPilotListWidget *weatherSnowList = new FrogPilotListWidget(this);

  ScrollView *advancedLongitudinalTunePanel = new ScrollView(advancedLongitudinalTuneList, this);
  ScrollView *aggressivePersonalityPanel = new ScrollView(aggressivePersonalityList, this);
  ScrollView *conditionalExperimentalPanel = new ScrollView(conditionalExperimentalList, this);
  ScrollView *curveSpeedPanel = new ScrollView(curveSpeedList, this);
  ScrollView *customDrivingPersonalityPanel = new ScrollView(customDrivingPersonalityList, this);
  ScrollView *longitudinalTunePanel = new ScrollView(longitudinalTuneList, this);
  ScrollView *qolPanel = new ScrollView(qolList, this);
  ScrollView *relaxedPersonalityPanel = new ScrollView(relaxedPersonalityList, this);
  ScrollView *speedLimitControllerPanel = new ScrollView(speedLimitControllerList, this);
  ScrollView *speedLimitControllerOffsetsPanel = new ScrollView(speedLimitControllerOffsetsList, this);
  ScrollView *speedLimitControllerQOLPanel = new ScrollView(speedLimitControllerQOLList, this);
  ScrollView *speedLimitControllerVisualPanel = new ScrollView(speedLimitControllerVisualList, this);
  ScrollView *standardPersonalityPanel = new ScrollView(standardPersonalityList, this);
  ScrollView *weatherLowVisibilityPanel = new ScrollView(weatherLowVisibilityList, this);
  ScrollView *weatherPanel = new ScrollView(weatherList, this);
  ScrollView *weatherRainPanel = new ScrollView(weatherRainList, this);
  ScrollView *weatherRainStormPanel = new ScrollView(weatherRainStormList, this);
  ScrollView *weatherSnowPanel = new ScrollView(weatherSnowList, this);

  longitudinalLayout->addWidget(advancedLongitudinalTunePanel);
  longitudinalLayout->addWidget(aggressivePersonalityPanel);
  longitudinalLayout->addWidget(conditionalExperimentalPanel);
  longitudinalLayout->addWidget(curveSpeedPanel);
  longitudinalLayout->addWidget(customDrivingPersonalityPanel);
  longitudinalLayout->addWidget(longitudinalTunePanel);
  longitudinalLayout->addWidget(qolPanel);
  longitudinalLayout->addWidget(relaxedPersonalityPanel);
  longitudinalLayout->addWidget(speedLimitControllerPanel);
  longitudinalLayout->addWidget(speedLimitControllerOffsetsPanel);
  longitudinalLayout->addWidget(speedLimitControllerQOLPanel);
  longitudinalLayout->addWidget(speedLimitControllerVisualPanel);
  longitudinalLayout->addWidget(standardPersonalityPanel);
  longitudinalLayout->addWidget(weatherLowVisibilityPanel);
  longitudinalLayout->addWidget(weatherPanel);
  longitudinalLayout->addWidget(weatherRainPanel);
  longitudinalLayout->addWidget(weatherRainStormPanel);
  longitudinalLayout->addWidget(weatherSnowPanel);

  const std::vector<std::tuple<QString, QString, QString, QString>> longitudinalToggles {
    {"AdvancedLongitudinalTune", tr("Advanced Longitudinal Tuning"), tr("<b>Fine-tune how openpilot accelerates and brakes with advanced settings for takeoff, standstill, and stopping behavior.</b>"), "../../frogpilot/assets/toggle_icons/icon_advanced_longitudinal_tune.png"},
    {"LongitudinalActuatorDelay", parent->longitudinalActuatorDelay != 0 ? QString(tr("Actuator Delay (Default: %1)")).arg(QString::number(parent->longitudinalActuatorDelay, 'f', 2)) : tr("Actuator Delay"), tr("<b>Sets how far ahead openpilot anticipates the lag between its throttle or brake command and the car actually responding.</b> Raise it if the car feels slow to start accelerating or braking. Lower it if it feels too eager or overshoots. The per-car default in the title is correct for almost everyone."), ""},
    {"MaxDesiredAcceleration", tr("Maximum Acceleration"), tr("<b>Caps how hard openpilot is allowed to accelerate, so the car never speeds up more strongly than this.</b> Lower it for gentler, slower pickup from stops and on-ramps. Raise it back toward the default for quicker acceleration.<br><br>Default: 4.0 m/s²."), ""},
    {"StartAccel", parent->startAccel != 0 ? QString(tr("Start Acceleration (Default: %1)")).arg(QString::number(parent->startAccel, 'f', 2)) : tr("Start Acceleration"), tr("<b>Sets how hard openpilot pulls away from a full stop.</b> The car learns this on its own, so the default suits almost every car. Only raise it if takeoffs from a stop feel sluggish, or lower it if they feel too abrupt."), ""},
    {"VEgoStarting", parent->vEgoStarting != 0 ? QString(tr("Start Speed (Default: %1)")).arg(QString::number(parent->vEgoStarting, 'f', 2)) : tr("Start Speed"), tr("<b>Sets the speed openpilot must reach before it treats itself as moving and ends the gentle start-from-stop ramp.</b> Raise it if the car commits to moving too eagerly after a stop. Lower it to pull away sooner and reduce creeping. The default is right for almost every car."), ""},
    {"StopAccel", parent->stopAccel != 0 ? QString(tr("Stop Acceleration (Default: %1)")).arg(QString::number(parent->stopAccel, 'f', 2)) : tr("Stop Acceleration"), tr("<b>Sets how hard openpilot brakes at the end of a stop and how firmly it holds the car still.</b> The car sets this on its own, so most drivers can leave it. Only change it if stops feel too abrupt (soften it toward 0) or the car creeps or rolls on hills (firm it up toward -4)."), ""},
    {"StoppingDecelRate", parent->stoppingDecelRate != 0 ? QString(tr("Stopping Rate (Default: %1)")).arg(QString::number(parent->stoppingDecelRate, 'f', 2)) : tr("Stopping Rate"), tr("<b>Sets how quickly braking firms up over the final moment of coming to a stop.</b> Raise it only if the last bit of the stop feels too soft or drawn-out. Lower it if the car finishes the stop too abruptly. Stick with the default unless one of those shows up."), ""},
    {"VEgoStopping", parent->vEgoStopping != 0 ? QString(tr("Stop Speed (Default: %1)")).arg(QString::number(parent->vEgoStopping, 'f', 2)) : tr("Stop Speed"), tr("<b>Sets the speed below which openpilot treats the car as stopped and commits to holding the brake.</b> Raise it to make openpilot finish the stop sooner and firmer. Lower it for a gentler creep-to-stop that can overshoot the stop point. The default works for nearly everyone."), ""},

    {"ConditionalExperimental", tr("Conditional Experimental Mode"), tr("<b>Automatically switch to \"Experimental Mode\" when set conditions are met, then revert once they clear.</b>"), "../../frogpilot/assets/toggle_icons/icon_conditional.png"},
    {"CESpeed", tr("No Lead Speed"), tr("<b>Switches to \"Experimental Mode\" when you drive below this speed with no lead car ahead.</b> Raise it to let the driving model handle more low-speed, no-lead situations. Lower it (or set 0 to turn off) to keep the normal planner in charge.<br><br>Default: 0 (off)."), ""},
    {"CECurves", tr("Curve Detected Ahead"), tr("<b>Switch to \"Experimental Mode\" whenever openpilot detects a curve ahead, then switch back afterward.</b> Turn this on if openpilot carries too much speed into curves. Leave it off if you prefer it to hold the set speed.<br><br>- \"With Lead\": Also trigger on curves while following a lead vehicle."), ""},
    {"CEStopLights", tr("\"Detected\" Stop Lights/Signs"), tr("<b>Switch to \"Experimental Mode\" whenever the driving model \"detects\" a red light or stop sign.</b><br><br><i><b>Disclaimer</b>: openpilot does not explicitly detect traffic lights or stop signs. In \"Experimental Mode\", openpilot makes end-to-end driving decisions from camera input, which means it may stop even when there's no clear reason!</i>"), ""},
    {"CELead", tr("Lead Detected Ahead"), tr("<b>Switch to \"Experimental Mode\" when a lead vehicle ahead slows down or stops, letting the driving model handle braking for it.</b> Turn this on if normal following braking feels late or abrupt. Leave it off to keep normal cruise behavior around lead vehicles.<br><br>- \"Slower Lead\": Trigger when the lead ahead is slowing down<br>- \"Stopped Lead\": Trigger when the lead ahead has stopped"), ""},
    {"CENavigation", tr("Navigation Maneuvers"), tr("<b>Switch to \"Experimental Mode\" when approaching intersections or turns on the active \"Navigate on openpilot\" (NOO) route.</b> This lets the driving model pick a smoother speed for the upcoming maneuver instead of holding the normal route speed.<br><br>- \"Intersections\": Trigger at mapped intersections<br>- \"Turns\": Trigger at mapped turns<br>- \"With Lead\": Also trigger when a lead vehicle is ahead"), ""},
    {"CEModelStopTime", tr("Predicted Stop In"), tr("<b>Switch to \"Experimental Mode\" when openpilot predicts a stop within the set time.</b> This is usually triggered when the model \"sees\" a red light or stop sign ahead.<br><br><i><b>Disclaimer</b>: openpilot does not explicitly detect traffic lights or stop signs. In \"Experimental Mode\", openpilot makes end-to-end driving decisions from camera input, which means it may stop even when there's no clear reason!</i>"), ""},
    {"CESignalSpeed", tr("Turn Signal Below"), tr("<b>Switch to \"Experimental Mode\" when you use a turn signal below this speed</b> so the driving model picks the pace through the turn. Raise it for faster turns, lower it for slow turns only, or set \"Off\" to disable.<br><br>- \"Not For Detected Lanes\": Skip signaled lane changes."), ""},
    {"ShowCEMStatus", tr("Experimental Mode Status Icon"), tr("<b>Show an on-screen icon indicating which condition triggered \"Experimental Mode\".</b> Turn ON to see at a glance why openpilot switched modes. Leave OFF for a cleaner driving screen."), ""},

    {"CurveSpeedController", tr("Curve Speed Controller"), tr("<b>Automatically slow down for upcoming curves, using lateral acceleration learned from how you take curves.</b>"), "../../frogpilot/assets/toggle_icons/icon_speed_map.png"},
    {"CalibratedLateralAcceleration", tr("Learned Cornering Grip"), tr("<b>The cornering grip openpilot has learned from your driving, which \"Curve Speed Controller\" uses to set curve speeds.</b> It updates automatically as you drive. A higher value means faster curves, and it starts at 2.00 m/s² before any data is collected."), ""},
    {"CalibrationProgress", tr("Calibration Progress"), tr("<b>How much curve data openpilot has collected to calibrate its cornering speeds.</b> It climbs as you drive curves of varying tightness, so it is normal for the value to stay low and rarely reach 100%."), ""},
    {"ResetCurveData", tr("Reset Curve Data"), tr("<b>Erase all learned curve data and make the \"Curve Speed Controller\" relearn your driving style from scratch.</b> Use this if its curve slowdowns feel wrong after a car or driving change. Behavior reverts to a generic baseline until openpilot re-collects data over your next drives."), ""},
    {"ShowCSCStatus", tr("Curve Speed Status Widget"), tr("<b>Show the \"Curve Speed Controller\" target speed on the driving screen, along with a learning indicator while it is still collecting curve data.</b> Turn this on to see when curves are slowing you down. Leave it off for a cleaner screen."), ""},

    {"CustomPersonalities", tr("Driving Personalities"), tr("<b>Customize the \"Driving Personalities\"</b> to better match your driving style."), "../../frogpilot/assets/toggle_icons/icon_personality.png"},

    {"AggressivePersonalityProfile", tr("Aggressive"), tr("<b>Customize the \"Aggressive\" personality profile.</b> Tuned for assertive driving with tighter gaps."), "../../frogpilot/assets/stock_theme/distance_icons/aggressive.png"},
    {"AggressiveFollow", tr("Following Distance"), tr("<b>How many seconds openpilot follows behind lead vehicles when using the \"Aggressive\" profile.</b> Increase it for more space. Decrease it for tighter gaps.<br><br>Default: 1.25 seconds."), ""},
    {"AggressiveJerkAcceleration", tr("Acceleration Smoothness"), tr("<b>How smoothly openpilot accelerates with the \"Aggressive\" profile.</b> Increase it for gentler starts. Decrease it for faster but more abrupt takeoffs."), ""},
    {"AggressiveJerkDeceleration", tr("Braking Smoothness"), tr("<b>How smoothly openpilot brakes with the \"Aggressive\" profile.</b> Increase it for gentler stops. Decrease it for quicker but sharper braking."), ""},
    {"AggressiveJerkDanger", tr("Safety Gap Bias"), tr("<b>How much extra space openpilot keeps from the vehicle ahead with the \"Aggressive\" profile.</b> Increase it for larger gaps and more cautious following. Decrease it for tighter gaps and closer following."), ""},
    {"AggressiveJerkSpeedDecrease", tr("Slowdown Response"), tr("<b>How smoothly openpilot slows down with the \"Aggressive\" profile.</b> Increase it for more gradual deceleration. Decrease it for faster but sharper slowdowns."), ""},
    {"AggressiveJerkSpeed", tr("Speed-Up Response"), tr("<b>How smoothly openpilot speeds up with the \"Aggressive\" profile.</b> Increase it for more gradual acceleration. Decrease it for quicker but more jolting acceleration."), ""},
    {"ResetAggressivePersonality", tr("Reset to Defaults"), tr("<b>Reset the \"Aggressive\" profile to defaults.</b>"), ""},

    {"StandardPersonalityProfile", tr("Standard"), tr("<b>Customize the \"Standard\" personality profile.</b> Best for balanced driving with moderate gaps."), "../../frogpilot/assets/stock_theme/distance_icons/standard.png"},
    {"StandardFollow", tr("Following Distance"), tr("<b>How many seconds openpilot follows behind lead vehicles when using the \"Standard\" profile.</b> Increase it for more space. Decrease it for tighter gaps.<br><br>Default: 1.45 seconds."), ""},
    {"StandardJerkAcceleration", tr("Acceleration Smoothness"), tr("<b>How smoothly openpilot accelerates with the \"Standard\" profile.</b> Increase it for gentler starts. Decrease it for faster but more abrupt takeoffs."), ""},
    {"StandardJerkDeceleration", tr("Braking Smoothness"), tr("<b>How smoothly openpilot brakes with the \"Standard\" profile.</b> Increase it for gentler stops. Decrease it for quicker but sharper braking."), ""},
    {"StandardJerkDanger", tr("Safety Gap Bias"), tr("<b>How much extra space openpilot keeps from the vehicle ahead with the \"Standard\" profile.</b> Increase it for larger gaps and more cautious following. Decrease it for tighter gaps and closer following."), ""},
    {"StandardJerkSpeedDecrease", tr("Slowdown Response"), tr("<b>How smoothly openpilot slows down with the \"Standard\" profile.</b> Increase it for more gradual deceleration. Decrease it for faster but sharper slowdowns."), ""},
    {"StandardJerkSpeed", tr("Speed-Up Response"), tr("<b>How smoothly openpilot speeds up with the \"Standard\" profile.</b> Increase it for more gradual acceleration. Decrease it for quicker but more jolting acceleration."), ""},
    {"ResetStandardPersonality", tr("Reset to Defaults"), tr("<b>Reset the \"Standard\" profile to defaults.</b>"), ""},

    {"RelaxedPersonalityProfile", tr("Relaxed"), tr("<b>Customize the \"Relaxed\" personality profile.</b> For smoother, more comfortable driving with larger gaps."), "../../frogpilot/assets/stock_theme/distance_icons/relaxed.png"},
    {"RelaxedFollow", tr("Following Distance"), tr("<b>How many seconds openpilot follows behind lead vehicles when using the \"Relaxed\" profile.</b> Increase it for more space. Decrease it for tighter gaps.<br><br>Default: 1.75 seconds."), ""},
    {"RelaxedJerkAcceleration", tr("Acceleration Smoothness"), tr("<b>How smoothly openpilot accelerates with the \"Relaxed\" profile.</b> Increase it for gentler starts. Decrease it for faster but more abrupt takeoffs."), ""},
    {"RelaxedJerkDeceleration", tr("Braking Smoothness"), tr("<b>How smoothly openpilot brakes with the \"Relaxed\" profile.</b> Increase it for gentler stops. Decrease it for quicker but sharper braking."), ""},
    {"RelaxedJerkDanger", tr("Safety Gap Bias"), tr("<b>How much extra space openpilot keeps from the vehicle ahead with the \"Relaxed\" profile.</b> Increase it for larger gaps and more cautious following. Decrease it for tighter gaps and closer following."), ""},
    {"RelaxedJerkSpeedDecrease", tr("Slowdown Response"), tr("<b>How smoothly openpilot slows down with the \"Relaxed\" profile.</b> Increase it for more gradual deceleration. Decrease it for faster but sharper slowdowns."), ""},
    {"RelaxedJerkSpeed", tr("Speed-Up Response"), tr("<b>How smoothly openpilot speeds up with the \"Relaxed\" profile.</b> Increase it for more gradual acceleration. Decrease it for quicker but more jolting acceleration."), ""},
    {"ResetRelaxedPersonality", tr("Reset to Defaults"), tr("<b>Reset the \"Relaxed\" profile to defaults.</b>"), ""},

    {"LongitudinalTune", tr("Longitudinal Tuning"), tr("<b>Adjust how openpilot accelerates, brakes, and follows lead vehicles.</b>"), "../../frogpilot/assets/toggle_icons/icon_longitudinal_tune.png"},
    {"AccelerationProfile", tr("Acceleration Profile"), tr("<b>How hard openpilot accelerates when speeding up.</b><br><br>- \"Standard\": openpilot's normal acceleration.<br>- \"Eco\": gentle and fuel-saving.<br>- \"Sport\": firmer and more responsive.<br>- \"Sport+\": accelerates at the maximum rate allowed."), ""},
    {"DecelerationProfile", tr("Deceleration Profile"), tr("<b>How firmly openpilot slows itself down when no car is ahead.</b> Pick a gentler profile if its braking feels too abrupt. Pick \"Standard\" if it slows too lazily and you want firmer, quicker stops.<br><br>- \"Standard\": Full braking power for the firmest, most decisive slowdowns<br>- \"Eco\": Coasts more and brakes gently<br>- \"Eco+\": Coasts the most and brakes as softly as possible<br><br>Default: Eco."), ""},
    {"HumanAcceleration", tr("Human-Like Acceleration"), tr("<b>openpilot accelerates more like a person</b> by easing onto the throttle at low speeds and shaping the launch when pulling away from a stop. Turn it ON for smoother, more natural-feeling acceleration. Leave it OFF for openpilot's standard, more uniform acceleration."), ""},
    {"HumanFollowing", tr("Human-Like Following"), tr("<b>Follows lead cars more like a human driver</b>, closing gaps sooner behind accelerating traffic for quicker takeoffs and easing the following distance for gentler braking. Leave on for more natural-feeling following. Turn off if you prefer openpilot's stock, more mechanical lead tracking."), ""},
    {"HumanLaneChanges", tr("Human-Like Lane Changes"), tr("<b>During a lane change, openpilot reacts to the car already in the lane it's moving into</b>, easing in behind it like a human checking the gap. Turn this on for smoother, more natural merges in traffic. Leave it off to keep openpilot focused on the lane it's leaving."), ""},
    {"LeadDetectionThreshold", tr("Lead Detection Confidence"), tr("<b>Sets how confident openpilot must be before it treats an object ahead as a lead car to follow.</b> Lower it if openpilot is slow to pick up real leads. Raise it if it brakes or slows for roadside objects that aren't cars. The default is fine for almost every driver.<br><br><i><b>Disclaimer</b>: lowering this makes openpilot accept weaker detections, so it may brake or slow for things that aren't actually vehicles.</i><br><br>Default: 35%."), ""},
    {"TacoTune", tr("Slow Down for Turns"), tr("<b>Proactively slows openpilot before and through curves and turns, easing off based on how sharp the corner is.</b> Turn ON if openpilot carries too much speed into bends and the turns feel rushed. Leave OFF to hold your set speed through corners."), ""},

    {"QOLLongitudinal", tr("Quality of Life"), tr("<b>Miscellaneous acceleration and braking control changes</b> to fine-tune how openpilot drives."), "../../frogpilot/assets/toggle_icons/icon_quality_of_life.png"},
    {"CustomCruise", tr("Cruise Interval"), tr("<b>How much the set speed changes for each tap of the + or - cruise control button.</b> Raise it to reach your speed in fewer presses. Lower it for finer, one-step-at-a-time adjustments.<br><br>Default: 1."), ""},
    {"CustomCruiseLong", tr("Cruise Interval (Hold)"), tr("<b>How much the set speed jumps with each step while you hold the + or - cruise button</b>, instead of the smaller single-tap step. Raise it to sweep to a new speed faster while holding. Lower it for finer control.<br><br>Default: 5."), ""},
    {"ForceStops", tr("Force Stop at Detected Lights/Signs"), tr("<b>Force openpilot to stop whenever the driving model \"detects\" a red light or stop sign.</b><br><br><i><b>Disclaimer</b>: openpilot does not explicitly detect traffic lights or stop signs. In \"Experimental Mode\", openpilot makes end-to-end driving decisions from camera input, which means it may stop even when there's no clear reason!</i>"), ""},
    {"IncreasedStoppedDistance", tr("Increase Stopped Distance by:"), tr("<b>Add extra space when stopped behind vehicles.</b> Increase it for more room. Decrease it for shorter gaps.<br><br>Default: 0 (no added distance)."), ""},
    {"MapGears", tr("Map Accel/Decel to Gears"), tr("<b>Tie openpilot's acceleration and braking to your car's physical \"Eco\" and \"Sport\" drive modes.</b> Enable the side you want the gear selector to control:<br><br>- \"Acceleration\": Eco speeds up gently, Sport speeds up harder.<br>- \"Deceleration\": Eco and Sport ease off and coast down more softly."), ""},
    {"SetSpeedOffset", tr("Set Speed Offset by:"), tr("<b>Shift openpilot's cruise set speed by this amount whenever you set it.</b> Raise it if you habitually cruise a few over your dialed-in speed. Leave it at 0 to set exactly what you choose.<br><br>Default: 0 (off)."), ""},
    {"ReverseCruise", tr("Reverse Cruise Increase"), tr("<b>Reverse the cruise control button behavior</b> so a short press changes the set speed by 5 and a long press by 1, instead of the other way around. Turn this on if you mostly make big speed changes and want fewer taps. Leave it off if you usually nudge the set speed in small steps."), ""},
    {"WeatherPresets", tr("Weather Offsets"), tr("<b>Adjust how openpilot drives in real-time weather, with separate offsets for low visibility, rain, rainstorms, and snow.</b>"), ""},

    {"LowVisibilityOffsets", tr("Low Visibility"), tr("<b>Driving adjustments for fog, haze, or other low-visibility conditions.</b>"), ""},
    {"IncreaseFollowingLowVisibility", tr("Increase Following Distance by:"), tr("<b>Add extra space behind lead vehicles in low visibility.</b> Increase it for more space. Decrease it for tighter gaps."), ""},
    {"IncreasedStoppedDistanceLowVisibility", tr("Increase Stopped Distance by:"), tr("<b>Add extra buffer when stopped behind vehicles in low visibility.</b> Increase it for more room. Decrease it for shorter gaps."), ""},
    {"ReduceAccelerationLowVisibility", tr("Reduce Acceleration by:"), tr("<b>Lower the maximum acceleration in low visibility.</b> Increase it for softer takeoffs. Decrease it for quicker but less stable takeoffs."), ""},
    {"ReduceLateralAccelerationLowVisibility", tr("Reduce Speed in Curves by:"), tr("<b>Lower the desired speed while driving through curves in low visibility.</b> Increase it for safer, gentler turns. Decrease it for more aggressive driving in curves."), ""},

    {"RainOffsets", tr("Rain"), tr("<b>Driving adjustments for rainy conditions.</b>"), ""},
    {"IncreaseFollowingRain", tr("Increase Following Distance by:"), tr("<b>Add extra space behind lead vehicles in rain.</b> Increase it for more space. Decrease it for tighter gaps."), ""},
    {"IncreasedStoppedDistanceRain", tr("Increase Stopped Distance by:"), tr("<b>Add extra buffer when stopped behind vehicles in rain.</b> Increase it for more room. Decrease it for shorter gaps."), ""},
    {"ReduceAccelerationRain", tr("Reduce Acceleration by:"), tr("<b>Lower the maximum acceleration in rain.</b> Increase it for softer takeoffs. Decrease it for quicker but less stable takeoffs."), ""},
    {"ReduceLateralAccelerationRain", tr("Reduce Speed in Curves by:"), tr("<b>Lower the desired speed while driving through curves in rain.</b> Increase it for safer, gentler turns. Decrease it for more aggressive driving in curves."), ""},

    {"RainStormOffsets", tr("Rainstorms"), tr("<b>Driving adjustments for rainstorms.</b>"), ""},
    {"IncreaseFollowingRainStorm", tr("Increase Following Distance by:"), tr("<b>Add extra space behind lead vehicles in a rainstorm.</b> Increase it for more space. Decrease it for tighter gaps."), ""},
    {"IncreasedStoppedDistanceRainStorm", tr("Increase Stopped Distance by:"), tr("<b>Add extra buffer when stopped behind vehicles in a rainstorm.</b> Increase it for more room. Decrease it for shorter gaps."), ""},
    {"ReduceAccelerationRainStorm", tr("Reduce Acceleration by:"), tr("<b>Lower the maximum acceleration in a rainstorm.</b> Increase it for softer takeoffs. Decrease it for quicker but less stable takeoffs."), ""},
    {"ReduceLateralAccelerationRainStorm", tr("Reduce Speed in Curves by:"), tr("<b>Lower the desired speed while driving through curves in a rainstorm.</b> Increase it for safer, gentler turns. Decrease it for more aggressive driving in curves."), ""},

    {"SnowOffsets", tr("Snow"), tr("<b>Driving adjustments for snowy conditions.</b>"), ""},
    {"IncreaseFollowingSnow", tr("Increase Following Distance by:"), tr("<b>Add extra space behind lead vehicles in snow.</b> Increase it for more space. Decrease it for tighter gaps."), ""},
    {"IncreasedStoppedDistanceSnow", tr("Increase Stopped Distance by:"), tr("<b>Add extra buffer when stopped behind vehicles in snow.</b> Increase it for more room. Decrease it for shorter gaps."), ""},
    {"ReduceAccelerationSnow", tr("Reduce Acceleration by:"), tr("<b>Lower the maximum acceleration in snow.</b> Increase it for softer takeoffs. Decrease it for quicker but less stable takeoffs."), ""},
    {"ReduceLateralAccelerationSnow", tr("Reduce Speed in Curves by:"), tr("<b>Lower the desired speed while driving through curves in snow.</b> Increase it for safer, gentler turns. Decrease it for more aggressive driving in curves."), ""},

    {"SetWeatherKey", tr("OpenWeatherMap Key"), tr("<b>Add your own \"OpenWeatherMap\" key to speed up how often the weather updates.</b><br><br><i>A personal key gives 1,000 free calls per day, so weather refreshes every minute. The shared default key only refreshes every 15 minutes.</i>"), ""},

    {"SpeedLimitController", tr("Speed Limit Controller"), tr("<b>Limit openpilot's maximum driving speed to the current posted speed limit</b>, read from downloaded map data, Navigate on openpilot, Mapbox, or the dashboard on supported vehicles."), "../../frogpilot/assets/toggle_icons/icon_speed_limit.png"},
    {"SLCFallback", tr("Fallback Speed"), tr("<b>The speed \"Speed Limit Controller\" targets when no speed limit is found for the road.</b> Pick the behavior you want where limits are unknown.<br><br>- \"Set Speed\": Run at your cruise set speed<br>- \"Experimental Mode\": Let openpilot's driving model choose the speed<br>- \"Previous Limit\": Keep using the last confirmed speed limit<br><br>Default: Previous Limit."), ""},
    {"SLCOverride", tr("Override Speed"), tr("<b>Which speed \"Speed Limit Controller\" holds after you tap the gas to go faster than the posted limit.</b><br><br>- \"None\": Ease back toward the posted limit once you let off the gas<br>- \"Set With Gas Pedal\": Hold the highest speed you reach while pressing the gas<br>- \"Max Set Speed\": Jump to your cruise set speed"), ""},
    {"SLCQOL", tr("Quality of Life"), tr("<b>Convenience options for the \"Speed Limit Controller\", like confirmation prompts, limit lookahead timing, and a Mapbox fallback.</b>"), ""},
    {"SLCConfirmation", tr("Confirm New Speed Limits"), tr("<b>Ask before openpilot changes its speed to a newly detected limit, instead of changing automatically.</b> Choose which direction needs your okay:<br><br>- \"Lower Limits\": Ask before slowing down for a reduced limit<br>- \"Higher Limits\": Ask before speeding up for a raised limit"), ""},
    {"ForceMPHDashboard", tr("Force MPH from Dashboard"), tr("<b>Always read dashboard speed limit signs in mph.</b> Turn this on if your cluster shows mph but \"Speed Limit Controller\" targets a much lower speed than the posted sign."), ""},
    {"SLCLookaheadHigher", tr("Higher Limit Lookahead Time"), tr("<b>How many seconds ahead openpilot starts speeding up for an upcoming higher speed limit from map data.</b> Raise it to begin accelerating sooner before the sign. Leave it at 0 to wait until the higher limit is reached.<br><br>Default: 0 seconds."), ""},
    {"SLCLookaheadLower", tr("Lower Limit Lookahead Time"), tr("<b>How many seconds ahead openpilot starts slowing for an upcoming lower speed limit from map data.</b> Raise it to begin easing off the gas sooner before the limit drops. Lower it to hold speed until you're closer.<br><br>Default: 0 seconds."), ""},
    {"SetSpeedLimit", tr("Match Speed Limit on Engage"), tr("<b>When openpilot first engages, set the cruise speed to the current posted limit instead of your current speed.</b> Turn this on if you want to start at the limit without dialing it in. Leave it off to keep openpilot starting from however fast you were already going."), ""},
    {"SLCMapboxFiller", tr("Fill Speed Limits with Mapbox"), tr("<b>Fill in the speed limit from Mapbox when the dashboard and downloaded maps have none.</b> Turn this on if your speed limit often goes blank in your area. Leave it off if another source usually has it."), ""},
    {"SLCPriority", tr("Speed Limit Source Priority"), tr("<b>Rank which sources openpilot trusts for posted speed limits when more than one reports a limit at the same spot.</b> openpilot uses the first source in your order that has a limit. Reorder them to favor the source you find most accurate.<br><br>- \"Dashboard\": The car's own speed-limit reader (supported cars only)<br>- \"Map Data\": Limits from OpenStreetMap (OSM)<br>- \"Navigation\": Limits from your active route<br>- \"Highest\" / \"Lowest\": Ignore order and always use the highest or lowest limit available"), ""},
    {"SLCOffsets", tr("Speed Limit Offsets"), tr("<b>Set how far above or below the posted speed limit openpilot cruises, per speed range.</b>"), ""},
    {"Offset1", tr("Speed Offset (0-24 mph)"), tr("<b>Adds to or subtracts from the posted speed limit in this range</b>, so openpilot cruises that much faster or slower than the sign. Use a positive value to flow with traffic above the limit, or a negative value to stay under it."), ""},
    {"Offset2", tr("Speed Offset (25-34 mph)"), tr("<b>Adds to or subtracts from the posted speed limit in this range</b>, so openpilot cruises that much faster or slower than the sign. Use a positive value to flow with traffic above the limit, or a negative value to stay under it."), ""},
    {"Offset3", tr("Speed Offset (35-44 mph)"), tr("<b>Adds to or subtracts from the posted speed limit in this range</b>, so openpilot cruises that much faster or slower than the sign. Use a positive value to flow with traffic above the limit, or a negative value to stay under it."), ""},
    {"Offset4", tr("Speed Offset (45-54 mph)"), tr("<b>Adds to or subtracts from the posted speed limit in this range</b>, so openpilot cruises that much faster or slower than the sign. Use a positive value to flow with traffic above the limit, or a negative value to stay under it."), ""},
    {"Offset5", tr("Speed Offset (55-64 mph)"), tr("<b>Adds to or subtracts from the posted speed limit in this range</b>, so openpilot cruises that much faster or slower than the sign. Use a positive value to flow with traffic above the limit, or a negative value to stay under it."), ""},
    {"Offset6", tr("Speed Offset (65-74 mph)"), tr("<b>Adds to or subtracts from the posted speed limit in this range</b>, so openpilot cruises that much faster or slower than the sign. Use a positive value to flow with traffic above the limit, or a negative value to stay under it."), ""},
    {"Offset7", tr("Speed Offset (75-99 mph)"), tr("<b>Adds to or subtracts from the posted speed limit in this range</b>, so openpilot cruises that much faster or slower than the sign. Use a positive value to flow with traffic above the limit, or a negative value to stay under it."), ""},
    {"SLCVisuals", tr("Speed Limit Display"), tr("<b>Choose what \"Speed Limit Controller\" info appears on the driving screen</b>, like the current offset from the posted limit and the speed-limit sources."), ""},
    {"ShowSLCOffset", tr("Show Speed Limit Offset"), tr("<b>Show your speed-limit offset as a separate value on the sign</b>, alongside the actual posted limit, instead of folding it into the displayed number. Turn this on when you want to see both the real posted limit and your offset at a glance. Leave it off to just see the single offset-adjusted speed."), ""},
    {"SpeedLimitSources", tr("Show Speed Limit Sources"), tr("<b>Display each speed-limit source and its current value on the driving screen, with the source openpilot is using highlighted.</b> Turn this on when you want to see where a limit is coming from or why the set limit looks wrong. Leave it off for a cleaner screen."), ""}
  };

  for (const auto &[param, title, desc, icon] : longitudinalToggles) {
    AbstractControl *longitudinalToggle;

    if (param == "AdvancedLongitudinalTune") {
      FrogPilotManageControl *advancedLongitudinalTuneToggle = new FrogPilotManageControl(param, title, desc, icon);
      QObject::connect(advancedLongitudinalTuneToggle, &FrogPilotManageControl::manageButtonClicked, [longitudinalLayout, advancedLongitudinalTunePanel]() {
        longitudinalLayout->setCurrentWidget(advancedLongitudinalTunePanel);
      });
      longitudinalToggle = advancedLongitudinalTuneToggle;
    } else if (param == "LongitudinalActuatorDelay") {
      longitudinalActuatorDelayToggle = new FrogPilotParamValueControl(param, title, desc, icon, 0, 1, tr(" seconds"), std::map<float, QString>(), 0.01);
      longitudinalToggle = longitudinalActuatorDelayToggle;
    } else if (param == "MaxDesiredAcceleration") {
      longitudinalToggle = new FrogPilotParamValueControl(param, title, desc, icon, 0.1, 4.0, tr(" m/s²"), std::map<float, QString>(), 0.1);
    } else if (param == "StartAccel") {
      startAccelToggle = new FrogPilotParamValueControl(param, title, desc, icon, 0, 4, tr(" m/s²"), std::map<float, QString>(), 0.01, true);
      longitudinalToggle = startAccelToggle;
    } else if (param == "VEgoStarting") {
      vEgoStartingToggle = new FrogPilotParamValueControl(param, title, desc, icon, 0.01, 1, tr(" m/s²"), std::map<float, QString>(), 0.01);
      longitudinalToggle = vEgoStartingToggle;
    } else if (param == "StopAccel") {
      stopAccelToggle = new FrogPilotParamValueControl(param, title, desc, icon, -4, 0, tr(" m/s²"), std::map<float, QString>(), 0.01, true);
      longitudinalToggle = stopAccelToggle;
    } else if (param == "StoppingDecelRate") {
      stoppingDecelRateToggle = new FrogPilotParamValueControl(param, title, desc, icon, 0.001, 1, tr(" m/s²"), std::map<float, QString>(), 0.001, true);
      longitudinalToggle = stoppingDecelRateToggle;
    } else if (param == "VEgoStopping") {
      vEgoStoppingToggle = new FrogPilotParamValueControl(param, title, desc, icon, 0.01, 1, tr(" m/s²"), std::map<float, QString>(), 0.01);
      longitudinalToggle = vEgoStoppingToggle;

    } else if (param == "ConditionalExperimental") {
      FrogPilotManageControl *conditionalExperimentalToggle = new FrogPilotManageControl(param, title, desc, icon);
      QObject::connect(conditionalExperimentalToggle, &FrogPilotManageControl::manageButtonClicked, [longitudinalLayout, conditionalExperimentalPanel]() {
        longitudinalLayout->setCurrentWidget(conditionalExperimentalPanel);
      });
      longitudinalToggle = conditionalExperimentalToggle;
    } else if (param == "CESpeed") {
      FrogPilotParamValueControl *CESpeed = new FrogPilotParamValueControl(param, title, desc, icon, 0, 99, tr(" mph"), std::map<float, QString>(), 1, true, 175);
      FrogPilotParamValueControl *CESpeedLead = new FrogPilotParamValueControl("CESpeedLead", tr("With Lead"), tr("<b>Switches to \"Experimental Mode\" when you drive below this speed with a lead car ahead.</b> Raise it to let the driving model handle more low-speed following in traffic. Lower it to hand control back to the normal planner sooner."), icon, 0, 99, tr(" mph"), std::map<float, QString>(), 1, true, 175);
      FrogPilotDualParamValueControl *conditionalSpeeds = new FrogPilotDualParamValueControl(CESpeed, CESpeedLead);
      longitudinalToggle = reinterpret_cast<AbstractControl*>(conditionalSpeeds);
    } else if (param == "CECurves") {
      std::vector<QString> curveToggles{"CECurvesLead"};
      std::vector<QString> curveToggleNames{tr("With Lead")};
      longitudinalToggle = new FrogPilotButtonToggleControl(param, title, desc, icon, curveToggles, curveToggleNames);
    } else if (param == "CELead") {
      std::vector<QString> leadToggles{"CESlowerLead", "CEStoppedLead"};
      std::vector<QString> leadToggleNames{tr("Slower Lead"), tr("Stopped Lead")};
      longitudinalToggle = new FrogPilotButtonToggleControl(param, title, desc, icon, leadToggles, leadToggleNames);
    } else if (param == "CENavigation") {
      std::vector<QString> navigationToggles{"CENavigationIntersections", "CENavigationTurns", "CENavigationLead"};
      std::vector<QString> navigationToggleNames{tr("Intersections"), tr("Turns"), tr("With Lead")};
      longitudinalToggle = new FrogPilotButtonToggleControl(param, title, desc, icon, navigationToggles, navigationToggleNames);
    } else if (param == "CEModelStopTime") {
      std::map<float, QString> stopTimeLabels;
      for (int i = 0; i <= 10; ++i) {
        stopTimeLabels[i] = i == 0 ? tr("Off") : i == 1 ? QString::number(i) + tr(" second") : QString::number(i) + tr(" seconds");
      }
      longitudinalToggle = new FrogPilotParamValueControl(param, title, desc, icon, 0, 9, QString(), stopTimeLabels);
    } else if (param == "CESignalSpeed") {
      std::vector<QString> ceSignalToggles{"CESignalLaneDetection"};
      std::vector<QString> ceSignalToggleNames{tr("Not For Detected Lanes")};
      longitudinalToggle = new FrogPilotParamValueButtonControl(param, title, desc, icon, 0, 99, tr(" mph"), std::map<float, QString>(), 1.0, true, ceSignalToggles, ceSignalToggleNames, true);

    } else if (param == "CurveSpeedController") {
      FrogPilotManageControl *curveControlToggle = new FrogPilotManageControl(param, title, desc, icon);
      QObject::connect(curveControlToggle, &FrogPilotManageControl::manageButtonClicked, [longitudinalLayout, curveSpeedPanel]() {
        longitudinalLayout->setCurrentWidget(curveSpeedPanel);
      });
      longitudinalToggle = curveControlToggle;
    } else if (param == "CalibrationProgress") {
      calibrationProgressLabel = new LabelControl(title, QString::number(params.getFloat("CalibrationProgress"), 'f', 2) + "%", desc);
      longitudinalToggle = calibrationProgressLabel;
    } else if (param == "CalibratedLateralAcceleration") {
      calibratedLateralAccelerationLabel = new LabelControl(title, QString::number(params.getFloat("CalibratedLateralAcceleration"), 'f', 2) + tr(" m/s²"), desc);
      longitudinalToggle = calibratedLateralAccelerationLabel;
    } else if (param == "ResetCurveData") {
      ButtonControl *resetCurveDataButton = new ButtonControl(title, tr("RESET"), desc);
      QObject::connect(resetCurveDataButton, &ButtonControl::clicked, [this]() {
        if (FrogPilotConfirmationDialog::yesorno(tr("Are you sure you want to completely reset your curvature data?"), this)) {
          params.putFloat("CalibratedLateralAcceleration", 2.00);
          params.remove("CalibrationProgress");
          params.remove("CurvatureData");

          params_cache.putFloat("CalibratedLateralAcceleration", 2.00);
          params_cache.remove("CalibrationProgress");
          params_cache.remove("CurvatureData");

          calibratedLateralAccelerationLabel->setText(QString::number(2.00, 'f', 2) + tr(" m/s²"));
          calibrationProgressLabel->setText(QString::number(0.00, 'f', 2) + "%");
        }
      });
      longitudinalToggle = resetCurveDataButton;

    } else if (param == "CustomPersonalities") {
      FrogPilotManageControl *customPersonalitiesToggle = new FrogPilotManageControl(param, title, desc, icon);
      QObject::connect(customPersonalitiesToggle, &FrogPilotManageControl::manageButtonClicked, [longitudinalLayout, customDrivingPersonalityPanel]() {
        longitudinalLayout->setCurrentWidget(customDrivingPersonalityPanel);
      });
      longitudinalToggle = customPersonalitiesToggle;
    } else if (param == "ResetAggressivePersonality" || param == "ResetStandardPersonality" || param == "ResetRelaxedPersonality") {
      ButtonControl *resetButton = new ButtonControl(title, tr("RESET"), desc);
      longitudinalToggle = resetButton;
    } else if (param == "AggressivePersonalityProfile") {
      FrogPilotManageControl *aggressivePersonalityToggle = new FrogPilotManageControl(param, title, desc, icon);
      QObject::connect(aggressivePersonalityToggle, &FrogPilotManageControl::manageButtonClicked, [longitudinalLayout, aggressivePersonalityPanel, this]() {
        openSubSubPanel();

        longitudinalLayout->setCurrentWidget(aggressivePersonalityPanel);

        customPersonalityOpen = true;
      });
      longitudinalToggle = aggressivePersonalityToggle;
    } else if (param == "StandardPersonalityProfile") {
      FrogPilotManageControl *standardPersonalityToggle = new FrogPilotManageControl(param, title, desc, icon);
      QObject::connect(standardPersonalityToggle, &FrogPilotManageControl::manageButtonClicked, [longitudinalLayout, standardPersonalityPanel, this]() {
        openSubSubPanel();

        longitudinalLayout->setCurrentWidget(standardPersonalityPanel);

        customPersonalityOpen = true;
      });
      longitudinalToggle = standardPersonalityToggle;
    } else if (param == "RelaxedPersonalityProfile") {
      FrogPilotManageControl *relaxedPersonalityToggle = new FrogPilotManageControl(param, title, desc, icon);
      QObject::connect(relaxedPersonalityToggle, &FrogPilotManageControl::manageButtonClicked, [longitudinalLayout, relaxedPersonalityPanel, this]() {
        openSubSubPanel();

        longitudinalLayout->setCurrentWidget(relaxedPersonalityPanel);

        customPersonalityOpen = true;
      });
      longitudinalToggle = relaxedPersonalityToggle;
    } else if (aggressivePersonalityKeys.contains(param) || standardPersonalityKeys.contains(param) || relaxedPersonalityKeys.contains(param)) {
      if (param == "AggressiveFollow" || param == "StandardFollow" || param == "RelaxedFollow") {
        std::map<float, QString> followTimeLabels;
        for (float i = 0; i <= 3; i += 0.01) {
          followTimeLabels[i] = std::lround(i / 0.01) == 1 / 0.01 ? QString::number(i, 'f', 2) + tr(" second") : QString::number(i, 'f', 2) + tr(" seconds");
        }
        longitudinalToggle = new FrogPilotParamValueControl(param, title, desc, icon, 1, 3, QString(), followTimeLabels, 0.01, true);
      } else {
        longitudinalToggle = new FrogPilotParamValueControl(param, title, desc, icon, 25, 200, "%");
      }

    } else if (param == "LongitudinalTune") {
      FrogPilotManageControl *longitudinalTuneToggle = new FrogPilotManageControl(param, title, desc, icon);
      QObject::connect(longitudinalTuneToggle, &FrogPilotManageControl::manageButtonClicked, [longitudinalLayout, longitudinalTunePanel]() {
        longitudinalLayout->setCurrentWidget(longitudinalTunePanel);
      });
      longitudinalToggle = longitudinalTuneToggle;
    } else if (param == "AccelerationProfile") {
      std::vector<QString> accelerationProfiles{tr("Standard"), tr("Eco"), tr("Sport"), tr("Sport+")};
      ButtonParamControl *accelerationProfileToggle = new ButtonParamControl(param, title, desc, icon, accelerationProfiles);
      longitudinalToggle = accelerationProfileToggle;
    } else if (param == "DecelerationProfile") {
      std::vector<QString> decelerationProfiles{tr("Standard"), tr("Eco"), tr("Eco+")};
      ButtonParamControl *decelerationProfileToggle = new ButtonParamControl(param, title, desc, icon, decelerationProfiles);
      longitudinalToggle = decelerationProfileToggle;
    } else if (param == "LeadDetectionThreshold") {
      longitudinalToggle = new FrogPilotParamValueControl(param, title, desc, icon, 25, 50, "%");

    } else if (param == "QOLLongitudinal") {
      FrogPilotManageControl *qolLongitudinalToggle = new FrogPilotManageControl(param, title, desc, icon);
      QObject::connect(qolLongitudinalToggle, &FrogPilotManageControl::manageButtonClicked, [longitudinalLayout, qolPanel]() {
        longitudinalLayout->setCurrentWidget(qolPanel);
      });
      longitudinalToggle = qolLongitudinalToggle;
    } else if (param == "CustomCruise") {
      longitudinalToggle = new FrogPilotParamValueControl(param, title, desc, icon, 1, 99, tr(" mph"));
    } else if (param == "CustomCruiseLong") {
      longitudinalToggle = new FrogPilotParamValueControl(param, title, desc, icon, 1, 99, tr(" mph"));
    } else if (param == "IncreasedStoppedDistance") {
      longitudinalToggle = new FrogPilotParamValueControl(param, title, desc, icon, 0, 10, tr(" feet"));
    } else if (param == "MapGears") {
      std::vector<QString> mapGearsToggles{"MapAcceleration", "MapDeceleration"};
      std::vector<QString> mapGearsToggleNames{tr("Acceleration"), tr("Deceleration")};
      longitudinalToggle = new FrogPilotButtonToggleControl(param, title, desc, icon, mapGearsToggles, mapGearsToggleNames);
    } else if (param == "SetSpeedOffset") {
      longitudinalToggle = new FrogPilotParamValueControl(param, title, desc, icon, 0, 99, tr(" mph"));
    } else if (param == "WeatherPresets") {
      FrogPilotManageControl *weatherToggle = new FrogPilotManageControl(param, title, desc, icon);
      QObject::connect(weatherToggle, &FrogPilotManageControl::manageButtonClicked, [longitudinalLayout, weatherPanel, this]() {
        openSubSubPanel();

        longitudinalLayout->setCurrentWidget(weatherPanel);

        qolOpen = true;
      });
      longitudinalToggle = weatherToggle;
    } else if (param == "SetWeatherKey") {
      weatherKeyControl = new FrogPilotButtonsControl(title, desc, icon, {tr("ADD"), tr("TEST")});
      QObject::connect(weatherKeyControl, &FrogPilotButtonsControl::buttonClicked, [this](int id) {
        if (id == 0) {
          if (!params.get("WeatherToken").empty()) {
            if (FrogPilotConfirmationDialog::yesorno(tr("Are you sure you want to remove your key?"), this)) {
              params.remove("WeatherToken");
              params_cache.remove("WeatherToken");

              weatherKeyControl->setText(0, tr("ADD"));
              weatherKeyControl->setVisibleButton(1, false);
            }
          } else {
            int keyLength = 32;
            QString currentKey = QString::fromStdString(params.get("WeatherToken"));
            QString newKey = InputDialog::getText(tr("Enter your \"OpenWeatherMap\" key"), this, tr("Characters: 0/%1").arg(keyLength), false, -1, currentKey, keyLength).trimmed();
            if (!newKey.isEmpty()) {
              params.put("WeatherToken", newKey.toStdString());

              weatherKeyControl->setText(0, tr("REMOVE"));
              weatherKeyControl->setVisibleButton(1, true);
            }
          }
        } else if (id == 1) {
          weatherKeyControl->setValue(tr("Testing..."));

          QString key = QString::fromStdString(params.get("WeatherToken")).trimmed();
          QString url30 = QString("https://api.openweathermap.org/data/3.0/onecall?lat=42.4293&lon=-83.9850&exclude=current,minutely,hourly,daily,alerts&appid=%1").arg(key);

          QNetworkRequest request(url30);
          QNetworkReply *reply = networkManager->get(request);
          QObject::connect(reply, &QNetworkReply::finished, this, [=]() {
            reply->deleteLater();

            if (reply->error() == QNetworkReply::NoError) {
              weatherKeyControl->setValue("");
              ConfirmationDialog::alert(tr("Key is valid."), this);
              return;
            }

            int status = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
            if (status == 401 || status == 403) {
              QString url25 = QString("https://api.openweathermap.org/data/2.5/weather?lat=42.4293&lon=-83.9850&appid=%1").arg(key);

              QNetworkRequest request25(url25);
              QNetworkReply *reply25 = networkManager->get(request25);
              QObject::connect(reply25, &QNetworkReply::finished, this, [=]() {
                reply25->deleteLater();

                weatherKeyControl->setValue("");
                if (reply25->error() == QNetworkReply::NoError) {
                  ConfirmationDialog::alert(tr("Your key works with version 2.5, but version 3.0 is recommended. Subscribe to the \"One Call API 3.0\" plan."), this);
                } else {
                   int status25 = reply25->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
                   ConfirmationDialog::alert(tr("Invalid key. (Error: %1)").arg(status25), this);
                }
              });
            } else {
              weatherKeyControl->setValue("");
              ConfirmationDialog::alert(tr("An error occurred: %1").arg(reply->errorString()), this);
            }
          });
        }
      });
      longitudinalToggle = weatherKeyControl;
    } else if (param == "LowVisibilityOffsets") {
      ButtonControl *manageLowVisibilitOffsetsButton = new ButtonControl(title, tr("MANAGE"), desc);
      QObject::connect(manageLowVisibilitOffsetsButton, &ButtonControl::clicked, [longitudinalLayout, weatherLowVisibilityPanel, this]() {
        openSubSubSubPanel();

        longitudinalLayout->setCurrentWidget(weatherLowVisibilityPanel);

        weatherOpen = true;
      });
      longitudinalToggle = manageLowVisibilitOffsetsButton;
    } else if (param == "RainOffsets") {
      ButtonControl *manageRainOffsetsButton = new ButtonControl(title, tr("MANAGE"), desc);
      QObject::connect(manageRainOffsetsButton, &ButtonControl::clicked, [longitudinalLayout, weatherRainPanel, this]() {
        openSubSubSubPanel();

        longitudinalLayout->setCurrentWidget(weatherRainPanel);

        weatherOpen = true;
      });
      longitudinalToggle = manageRainOffsetsButton;
    } else if (param == "RainStormOffsets") {
      ButtonControl *manageRainStormOffsetsButton = new ButtonControl(title, tr("MANAGE"), desc);
      QObject::connect(manageRainStormOffsetsButton, &ButtonControl::clicked, [longitudinalLayout, weatherRainStormPanel, this]() {
        openSubSubSubPanel();

        longitudinalLayout->setCurrentWidget(weatherRainStormPanel);

        weatherOpen = true;
      });
      longitudinalToggle = manageRainStormOffsetsButton;
    } else if (param == "SnowOffsets") {
      ButtonControl *manageSnowOffsetsButton = new ButtonControl(title, tr("MANAGE"), desc);
      QObject::connect(manageSnowOffsetsButton, &ButtonControl::clicked, [longitudinalLayout, weatherSnowPanel, this]() {
        openSubSubSubPanel();

        longitudinalLayout->setCurrentWidget(weatherSnowPanel);

        weatherOpen = true;
      });
      longitudinalToggle = manageSnowOffsetsButton;
    } else if (param == "IncreaseFollowingLowVisibility" || param == "IncreaseFollowingRain" || param == "IncreaseFollowingRainStorm" || param == "IncreaseFollowingSnow") {
      std::map<float, QString> followTimeLabels;
      for (float i = 0; i <= 3; i += 0.01) {
        followTimeLabels[i] = std::lround(i / 0.01) == 1 / 0.01 ? QString::number(i, 'f', 2) + tr(" second") : QString::number(i, 'f', 2) + tr(" seconds");
      }
      longitudinalToggle = new FrogPilotParamValueControl(param, title, desc, icon, 0, 3, QString(), followTimeLabels, 0.01, true);
    } else if (param == "IncreasedStoppedDistanceLowVisibility" || param == "IncreasedStoppedDistanceRain" || param == "IncreasedStoppedDistanceRainStorm" || param == "IncreasedStoppedDistanceSnow") {
      longitudinalToggle = new FrogPilotParamValueControl(param, title, desc, icon, 0, 10, tr(" feet"));
    } else if (param == "ReduceAccelerationLowVisibility" || param == "ReduceAccelerationRain" || param == "ReduceAccelerationRainStorm" || param == "ReduceAccelerationSnow") {
      longitudinalToggle = new FrogPilotParamValueControl(param, title, desc, icon, 0, 99, "%", std::map<float, QString>(), 1);
    } else if (param == "ReduceLateralAccelerationLowVisibility" || param == "ReduceLateralAccelerationRain" || param == "ReduceLateralAccelerationRainStorm" || param == "ReduceLateralAccelerationSnow") {
      longitudinalToggle = new FrogPilotParamValueControl(param, title, desc, icon, 0, 99, "%", std::map<float, QString>(), 1);

    } else if (param == "SpeedLimitController") {
      FrogPilotManageControl *speedLimitControllerToggle = new FrogPilotManageControl(param, title, desc, icon);
      QObject::connect(speedLimitControllerToggle, &FrogPilotManageControl::manageButtonClicked, [longitudinalLayout, speedLimitControllerPanel]() {
        longitudinalLayout->setCurrentWidget(speedLimitControllerPanel);
      });
      longitudinalToggle = speedLimitControllerToggle;
    } else if (param == "SLCFallback") {
      std::vector<QString> fallbackOptions{tr("Set Speed"), tr("Experimental Mode"), tr("Previous Limit")};
      ButtonParamControl *fallbackSelection = new ButtonParamControl(param, title, desc, icon, fallbackOptions);
      longitudinalToggle = fallbackSelection;
    } else if (param == "SLCOverride") {
      std::vector<QString> overrideOptions{tr("None"), tr("Set With Gas Pedal"), tr("Max Set Speed")};
      ButtonParamControl *overrideSelection = new ButtonParamControl(param, title, desc, icon, overrideOptions);
      longitudinalToggle = overrideSelection;
    } else if (param == "SLCPriority") {
      ButtonControl *slcPriorityButton = new ButtonControl(title, tr("SELECT"), desc);
      QStringList primaryPriorities = {tr("Dashboard"), tr("Map Data"), tr("Navigation"), tr("Highest"), tr("Lowest")};
      QStringList otherPriorities = {tr("None"), tr("Dashboard"), tr("Map Data"), tr("Navigation")};
      QStringList priorityPrompts = {tr("Select your primary priority"), tr("Select your secondary priority"), tr("Select your tertiary priority")};

      QObject::connect(slcPriorityButton, &ButtonControl::clicked, [=]() {
        QStringList selectedPriorities;

        for (int i = 1; i <= 3; ++i) {
          QStringList availablePriorities = i == 1 ? primaryPriorities : otherPriorities;
          availablePriorities = availablePriorities.toSet().subtract(selectedPriorities.toSet()).toList();

          if (!parent->hasDashSpeedLimits) {
            availablePriorities.removeAll(tr("Dashboard"));
          }
          if (availablePriorities.size() == 1 && availablePriorities.contains(tr("None"))) {
            break;
          }

          QString selection = MultiOptionDialog::getSelection(priorityPrompts[i - 1], availablePriorities, "", this);
          if (selection.isEmpty()) {
            break;
          }

          selectedPriorities.append(selection);

          params.put(QString("SLCPriority%1").arg(i).toStdString(), selection.toStdString());
          if (selection == tr("None")) {
            for (int j = i + 1; j <= 3; ++j) {
              params.put(QString("SLCPriority%1").arg(j).toStdString(), tr("None").toStdString());
            }
            break;
          }

          if (selection == tr("Lowest") || selection == tr("Highest")) {
            break;
          }
        }

        selectedPriorities.removeAll(tr("None"));
        if (!selectedPriorities.isEmpty()) {
          slcPriorityButton->setValue(selectedPriorities.join(", "));
        }
      });

      QStringList selectedPriorities;
      for (int i = 1; i <= 3; ++i) {
        QString priority = QString::fromStdString(params.get(QString("SLCPriority%1").arg(i).toStdString()));
        if (primaryPriorities.contains(priority)) {
          selectedPriorities.append(priority);
        }
      }
      slcPriorityButton->setValue(selectedPriorities.join(", "));

      longitudinalToggle = slcPriorityButton;
    } else if (param == "SLCOffsets") {
      ButtonControl *manageSLCOffsetsButton = new ButtonControl(title, tr("MANAGE"), desc);
      QObject::connect(manageSLCOffsetsButton, &ButtonControl::clicked, [longitudinalLayout, speedLimitControllerOffsetsPanel, this]() {
        openSubSubPanel();

        longitudinalLayout->setCurrentWidget(speedLimitControllerOffsetsPanel);

        slcOpen = true;
      });
      longitudinalToggle = manageSLCOffsetsButton;
    } else if (speedLimitControllerOffsetsKeys.contains(param)) {
      longitudinalToggle = new FrogPilotParamValueControl(param, title, desc, icon, -99, 99, tr(" mph"));
    } else if (param == "SLCQOL") {
      ButtonControl *manageSLCQOLButton = new ButtonControl(title, tr("MANAGE"), desc);
      QObject::connect(manageSLCQOLButton, &ButtonControl::clicked, [longitudinalLayout, speedLimitControllerQOLPanel, this]() {
        openSubSubPanel();

        longitudinalLayout->setCurrentWidget(speedLimitControllerQOLPanel);

        slcOpen = true;
      });
      longitudinalToggle = manageSLCQOLButton;
    } else if (param == "SLCConfirmation") {
      std::vector<QString> confirmationToggles{"SLCConfirmationLower", "SLCConfirmationHigher"};
      std::vector<QString> confirmationToggleNames{tr("Lower Limits"), tr("Higher Limits")};
      longitudinalToggle = new FrogPilotButtonToggleControl(param, title, desc, icon, confirmationToggles, confirmationToggleNames);
    } else if (param == "SLCLookaheadHigher" || param == "SLCLookaheadLower") {
      longitudinalToggle = new FrogPilotParamValueControl(param, title, desc, icon, 0, 30, tr(" seconds"));
    } else if (param == "SLCVisuals") {
      ButtonControl *manageSLCVisualsButton = new ButtonControl(title, tr("MANAGE"), desc);
      QObject::connect(manageSLCVisualsButton, &ButtonControl::clicked, [longitudinalLayout, speedLimitControllerVisualPanel, this]() {
        openSubSubPanel();

        longitudinalLayout->setCurrentWidget(speedLimitControllerVisualPanel);

        slcOpen = true;
      });
      longitudinalToggle = manageSLCVisualsButton;

    } else {
      longitudinalToggle = new ParamControl(param, title, desc, icon);
    }

    toggles[param] = longitudinalToggle;

    if (advancedLongitudinalTuneKeys.contains(param)) {
      advancedLongitudinalTuneList->addItem(longitudinalToggle);
    } else if (aggressivePersonalityKeys.contains(param)) {
      aggressivePersonalityList->addItem(longitudinalToggle);
    } else if (conditionalExperimentalKeys.contains(param)) {
      conditionalExperimentalList->addItem(longitudinalToggle);
    } else if (curveSpeedKeys.contains(param)) {
      curveSpeedList->addItem(longitudinalToggle);
    } else if (customDrivingPersonalityKeys.contains(param)) {
      customDrivingPersonalityList->addItem(longitudinalToggle);
    } else if (longitudinalTuneKeys.contains(param)) {
      longitudinalTuneList->addItem(longitudinalToggle);
    } else if (qolKeys.contains(param)) {
      qolList->addItem(longitudinalToggle);
    } else if (relaxedPersonalityKeys.contains(param)) {
      relaxedPersonalityList->addItem(longitudinalToggle);
    } else if (speedLimitControllerKeys.contains(param)) {
      speedLimitControllerList->addItem(longitudinalToggle);
    } else if (speedLimitControllerOffsetsKeys.contains(param)) {
      speedLimitControllerOffsetsList->addItem(longitudinalToggle);
    } else if (speedLimitControllerQOLKeys.contains(param)) {
      speedLimitControllerQOLList->addItem(longitudinalToggle);
    } else if (speedLimitControllerVisualKeys.contains(param)) {
      speedLimitControllerVisualList->addItem(longitudinalToggle);
    } else if (standardPersonalityKeys.contains(param)) {
      standardPersonalityList->addItem(longitudinalToggle);
    } else if (weatherKeys.contains(param)) {
      weatherList->addItem(longitudinalToggle);
    } else if (weatherLowVisibilityKeys.contains(param)) {
      weatherLowVisibilityList->addItem(longitudinalToggle);
    } else if (weatherRainKeys.contains(param)) {
      weatherRainList->addItem(longitudinalToggle);
    } else if (weatherRainStormKeys.contains(param)) {
      weatherRainStormList->addItem(longitudinalToggle);
    } else if (weatherSnowKeys.contains(param)) {
      weatherSnowList->addItem(longitudinalToggle);
    } else {
      longitudinalList->addItem(longitudinalToggle);

      parentKeys.insert(param);
    }

    if (FrogPilotManageControl *frogPilotManageToggle = qobject_cast<FrogPilotManageControl*>(longitudinalToggle)) {
      QObject::connect(frogPilotManageToggle, &FrogPilotManageControl::manageButtonClicked, [this]() {
        emit openSubPanel();
        openDescriptions(forceOpenDescriptions, toggles);
      });
    }

    QObject::connect(longitudinalToggle, &AbstractControl::hideDescriptionEvent, [this]() {
      update();
    });
    QObject::connect(longitudinalToggle, &AbstractControl::showDescriptionEvent, [this]() {
      update();
    });
  }

  QSet<QString> forceUpdateKeys = {"HumanAcceleration", "LongitudinalTune"};
  for (const QString &key : forceUpdateKeys) {
    QObject::connect(static_cast<ToggleControl*>(toggles[key]), &ToggleControl::toggleFlipped, this, &FrogPilotLongitudinalPanel::updateToggles);
  }

  FrogPilotParamValueControl *aggressiveFollowToggle = static_cast<FrogPilotParamValueControl*>(toggles["AggressiveFollow"]);
  FrogPilotParamValueControl *aggressiveAccelerationToggle = static_cast<FrogPilotParamValueControl*>(toggles["AggressiveJerkAcceleration"]);
  FrogPilotParamValueControl *aggressiveDecelerationToggle = static_cast<FrogPilotParamValueControl*>(toggles["AggressiveJerkDeceleration"]);
  FrogPilotParamValueControl *aggressiveDangerToggle = static_cast<FrogPilotParamValueControl*>(toggles["AggressiveJerkDanger"]);
  FrogPilotParamValueControl *aggressiveSpeedToggle = static_cast<FrogPilotParamValueControl*>(toggles["AggressiveJerkSpeed"]);
  FrogPilotParamValueControl *aggressiveSpeedDecreaseToggle = static_cast<FrogPilotParamValueControl*>(toggles["AggressiveJerkSpeedDecrease"]);
  FrogPilotButtonsControl *aggressiveResetButton = static_cast<FrogPilotButtonsControl*>(toggles["ResetAggressivePersonality"]);
  QObject::connect(aggressiveResetButton, &FrogPilotButtonsControl::buttonClicked, [=]() {
    if (FrogPilotConfirmationDialog::yesorno(tr("Are you sure you want to completely reset your settings for the <b>Aggressive</b> personality?"), this)) {
      params.putFloat("AggressiveFollow", params_default.getFloat("AggressiveFollow"));
      params.putFloat("AggressiveJerkAcceleration", params_default.getFloat("AggressiveJerkAcceleration"));
      params.putFloat("AggressiveJerkDeceleration", params_default.getFloat("AggressiveJerkDeceleration"));
      params.putFloat("AggressiveJerkDanger", params_default.getFloat("AggressiveJerkDanger"));
      params.putFloat("AggressiveJerkSpeed", params_default.getFloat("AggressiveJerkSpeed"));
      params.putFloat("AggressiveJerkSpeedDecrease", params_default.getFloat("AggressiveJerkSpeedDecrease"));

      aggressiveFollowToggle->refresh();
      aggressiveAccelerationToggle->refresh();
      aggressiveDecelerationToggle->refresh();
      aggressiveDangerToggle->refresh();
      aggressiveSpeedToggle->refresh();
      aggressiveSpeedDecreaseToggle->refresh();
    }
  });

  FrogPilotParamValueControl *standardFollowToggle = static_cast<FrogPilotParamValueControl*>(toggles["StandardFollow"]);
  FrogPilotParamValueControl *standardAccelerationToggle = static_cast<FrogPilotParamValueControl*>(toggles["StandardJerkAcceleration"]);
  FrogPilotParamValueControl *standardDecelerationToggle = static_cast<FrogPilotParamValueControl*>(toggles["StandardJerkDeceleration"]);
  FrogPilotParamValueControl *standardDangerToggle = static_cast<FrogPilotParamValueControl*>(toggles["StandardJerkDanger"]);
  FrogPilotParamValueControl *standardSpeedToggle = static_cast<FrogPilotParamValueControl*>(toggles["StandardJerkSpeed"]);
  FrogPilotParamValueControl *standardSpeedDecreaseToggle = static_cast<FrogPilotParamValueControl*>(toggles["StandardJerkSpeedDecrease"]);
  FrogPilotButtonsControl *standardResetButton = static_cast<FrogPilotButtonsControl*>(toggles["ResetStandardPersonality"]);
  QObject::connect(standardResetButton, &FrogPilotButtonsControl::buttonClicked, [=]() {
    if (FrogPilotConfirmationDialog::yesorno(tr("Are you sure you want to completely reset your settings for the <b>Standard</b> personality?"), this)) {
      params.putFloat("StandardFollow", params_default.getFloat("StandardFollow"));
      params.putFloat("StandardJerkAcceleration", params_default.getFloat("StandardJerkAcceleration"));
      params.putFloat("StandardJerkDeceleration", params_default.getFloat("StandardJerkDeceleration"));
      params.putFloat("StandardJerkDanger", params_default.getFloat("StandardJerkDanger"));
      params.putFloat("StandardJerkSpeed", params_default.getFloat("StandardJerkSpeed"));
      params.putFloat("StandardJerkSpeedDecrease", params_default.getFloat("StandardJerkSpeedDecrease"));

      standardFollowToggle->refresh();
      standardAccelerationToggle->refresh();
      standardDecelerationToggle->refresh();
      standardDangerToggle->refresh();
      standardSpeedToggle->refresh();
      standardSpeedDecreaseToggle->refresh();
    }
  });

  FrogPilotParamValueControl *relaxedFollowToggle = static_cast<FrogPilotParamValueControl*>(toggles["RelaxedFollow"]);
  FrogPilotParamValueControl *relaxedAccelerationToggle = static_cast<FrogPilotParamValueControl*>(toggles["RelaxedJerkAcceleration"]);
  FrogPilotParamValueControl *relaxedDecelerationToggle = static_cast<FrogPilotParamValueControl*>(toggles["RelaxedJerkDeceleration"]);
  FrogPilotParamValueControl *relaxedDangerToggle = static_cast<FrogPilotParamValueControl*>(toggles["RelaxedJerkDanger"]);
  FrogPilotParamValueControl *relaxedSpeedToggle = static_cast<FrogPilotParamValueControl*>(toggles["RelaxedJerkSpeed"]);
  FrogPilotParamValueControl *relaxedSpeedDecreaseToggle = static_cast<FrogPilotParamValueControl*>(toggles["RelaxedJerkSpeedDecrease"]);
  FrogPilotButtonsControl *relaxedResetButton = static_cast<FrogPilotButtonsControl*>(toggles["ResetRelaxedPersonality"]);
  QObject::connect(relaxedResetButton, &FrogPilotButtonsControl::buttonClicked, [=]() {
    if (FrogPilotConfirmationDialog::yesorno(tr("Are you sure you want to completely reset your settings for the <b>Relaxed</b> personality?"), this)) {
      params.putFloat("RelaxedFollow", params_default.getFloat("RelaxedFollow"));
      params.putFloat("RelaxedJerkAcceleration", params_default.getFloat("RelaxedJerkAcceleration"));
      params.putFloat("RelaxedJerkDeceleration", params_default.getFloat("RelaxedJerkDeceleration"));
      params.putFloat("RelaxedJerkDanger", params_default.getFloat("RelaxedJerkDanger"));
      params.putFloat("RelaxedJerkSpeed", params_default.getFloat("RelaxedJerkSpeed"));
      params.putFloat("RelaxedJerkSpeedDecrease", params_default.getFloat("RelaxedJerkSpeedDecrease"));

      relaxedFollowToggle->refresh();
      relaxedAccelerationToggle->refresh();
      relaxedDecelerationToggle->refresh();
      relaxedDangerToggle->refresh();
      relaxedSpeedToggle->refresh();
      relaxedSpeedDecreaseToggle->refresh();
    }
  });

  openDescriptions(forceOpenDescriptions, toggles);

  QObject::connect(parent, &FrogPilotSettingsWindow::closeSubPanel, [longitudinalLayout, longitudinalPanel, this] {
    openDescriptions(forceOpenDescriptions, toggles);
    longitudinalLayout->setCurrentWidget(longitudinalPanel);
  });
  QObject::connect(parent, &FrogPilotSettingsWindow::closeSubSubPanel, [longitudinalLayout, customDrivingPersonalityPanel, qolPanel, speedLimitControllerPanel, this]() {
    openDescriptions(forceOpenDescriptions, toggles);

    if (customPersonalityOpen) {
      longitudinalLayout->setCurrentWidget(customDrivingPersonalityPanel);

      customPersonalityOpen = false;
    } else if (qolOpen) {
      longitudinalLayout->setCurrentWidget(qolPanel);

      qolOpen = false;
    } else if (slcOpen) {
      longitudinalLayout->setCurrentWidget(speedLimitControllerPanel);

      slcOpen = false;
    }
  });
  QObject::connect(parent, &FrogPilotSettingsWindow::closeSubSubSubPanel, [longitudinalLayout, weatherPanel, this]() {
    openDescriptions(forceOpenDescriptions, toggles);

    if (weatherOpen) {
      longitudinalLayout->setCurrentWidget(weatherPanel);

      weatherOpen = false;
    }
  });
  QObject::connect(parent, &FrogPilotSettingsWindow::updateMetric, this, &FrogPilotLongitudinalPanel::updateMetric);
}

void FrogPilotLongitudinalPanel::showEvent(QShowEvent *event) {
  FrogPilotUIState &fs = *frogpilotUIState();

  frogpilotToggleLevels = parent->frogpilotToggleLevels;

  calibratedLateralAccelerationLabel->setText(QString::number(params.getFloat("CalibratedLateralAcceleration"), 'f', 2) + tr(" m/s²"));
  calibrationProgressLabel->setText(QString::number(params.getFloat("CalibrationProgress"), 'f', 2) + "%");

  longitudinalActuatorDelayToggle->setTitle(QString(tr("Actuator Delay (Default: %1)")).arg(QString::number(parent->longitudinalActuatorDelay, 'f', 2)));
  startAccelToggle->setTitle(QString(tr("Start Acceleration (Default: %1)")).arg(QString::number(parent->startAccel, 'f', 2)));
  stopAccelToggle->setTitle(QString(tr("Stop Acceleration (Default: %1)")).arg(QString::number(parent->stopAccel, 'f', 2)));
  stoppingDecelRateToggle->setTitle(QString(tr("Stopping Rate (Default: %1)")).arg(QString::number(parent->stoppingDecelRate, 'f', 2)));
  vEgoStartingToggle->setTitle(QString(tr("Start Speed (Default: %1)")).arg(QString::number(parent->vEgoStarting, 'f', 2)));
  vEgoStoppingToggle->setTitle(QString(tr("Stop Speed (Default: %1)")).arg(QString::number(parent->vEgoStopping, 'f', 2)));

  bool keyExists = !params.get("WeatherToken").empty();
  weatherKeyControl->setText(0, keyExists ? tr("REMOVE") : tr("ADD"));
  weatherKeyControl->setVisibleButton(1, keyExists && fs.frogpilot_scene.online);

  updateToggles();
}

void FrogPilotLongitudinalPanel::updateMetric(bool metric, bool bootRun) {
  static bool previousMetric;
  if (metric != previousMetric && !bootRun) {
    double distanceConversion = metric ? FOOT_TO_METER : METER_TO_FOOT;
    double speedConversion = metric ? MILE_TO_KM : KM_TO_MILE;

    params.putIntNonBlocking("IncreasedStoppedDistance", params.getInt("IncreasedStoppedDistance") * distanceConversion);
    params.putIntNonBlocking("IncreasedStoppedDistanceLowVisibility", params.getInt("IncreasedStoppedDistanceLowVisibility") * distanceConversion);
    params.putIntNonBlocking("IncreasedStoppedDistanceRain", params.getInt("IncreasedStoppedDistanceRain") * distanceConversion);
    params.putIntNonBlocking("IncreasedStoppedDistanceRainStorm", params.getInt("IncreasedStoppedDistanceRainStorm") * distanceConversion);
    params.putIntNonBlocking("IncreasedStoppedDistanceSnow", params.getInt("IncreasedStoppedDistanceSnow") * distanceConversion);

    params.putIntNonBlocking("CESignalSpeed", params.getInt("CESignalSpeed") * speedConversion);
    params.putIntNonBlocking("CESpeed", params.getInt("CESpeed") * speedConversion);
    params.putIntNonBlocking("CESpeedLead", params.getInt("CESpeedLead") * speedConversion);
    params.putIntNonBlocking("CustomCruise", params.getInt("CustomCruise") * speedConversion);
    params.putIntNonBlocking("CustomCruiseLong", params.getInt("CustomCruiseLong") * speedConversion);
    params.putIntNonBlocking("Offset1", params.getInt("Offset1") * speedConversion);
    params.putIntNonBlocking("Offset2", params.getInt("Offset2") * speedConversion);
    params.putIntNonBlocking("Offset3", params.getInt("Offset3") * speedConversion);
    params.putIntNonBlocking("Offset4", params.getInt("Offset4") * speedConversion);
    params.putIntNonBlocking("Offset5", params.getInt("Offset5") * speedConversion);
    params.putIntNonBlocking("Offset6", params.getInt("Offset6") * speedConversion);
    params.putIntNonBlocking("Offset7", params.getInt("Offset7") * speedConversion);
    params.putIntNonBlocking("SetSpeedOffset", params.getInt("SetSpeedOffset") * speedConversion);
  }
  previousMetric = metric;

  static std::map<float, QString> imperialDistanceLabels;
  static std::map<float, QString> imperialSpeedLabels;
  static std::map<float, QString> metricDistanceLabels;
  static std::map<float, QString> metricSpeedLabels;

  static bool labelsInitialized = false;
  if (!labelsInitialized) {
    for (int i = 0; i <= 10; ++i) {
      imperialDistanceLabels[i] = i == 0 ? tr("Off") : i == 1 ? QString::number(i) + tr(" foot") : QString::number(i) + tr(" feet");
    }

    for (int i = -99; i <= 99; ++i) {
      imperialSpeedLabels[i] = i == 0 ? tr("Off") : QString::number(i) + tr(" mph");
    }

    for (int i = 0; i <= 3; ++i) {
      metricDistanceLabels[i] = i == 0 ? tr("Off") : i == 1 ? QString::number(i) + tr(" meter") : QString::number(i) + tr(" meters");
    }

    for (int i = -150; i <= 150; ++i) {
      metricSpeedLabels[i] = i == 0 ? tr("Off") : QString::number(i) + tr(" km/h");
    }

    labelsInitialized = true;
  }

  FrogPilotDualParamValueControl *ceSpeedToggle = reinterpret_cast<FrogPilotDualParamValueControl*>(toggles["CESpeed"]);
  FrogPilotParamValueButtonControl *ceSignal = static_cast<FrogPilotParamValueButtonControl*>(toggles["CESignalSpeed"]);
  FrogPilotParamValueControl *customCruiseToggle = static_cast<FrogPilotParamValueControl*>(toggles["CustomCruise"]);
  FrogPilotParamValueControl *customCruiseLongToggle = static_cast<FrogPilotParamValueControl*>(toggles["CustomCruiseLong"]);
  FrogPilotParamValueControl *offset1Toggle = static_cast<FrogPilotParamValueControl*>(toggles["Offset1"]);
  FrogPilotParamValueControl *offset2Toggle = static_cast<FrogPilotParamValueControl*>(toggles["Offset2"]);
  FrogPilotParamValueControl *offset3Toggle = static_cast<FrogPilotParamValueControl*>(toggles["Offset3"]);
  FrogPilotParamValueControl *offset4Toggle = static_cast<FrogPilotParamValueControl*>(toggles["Offset4"]);
  FrogPilotParamValueControl *offset5Toggle = static_cast<FrogPilotParamValueControl*>(toggles["Offset5"]);
  FrogPilotParamValueControl *offset6Toggle = static_cast<FrogPilotParamValueControl*>(toggles["Offset6"]);
  FrogPilotParamValueControl *offset7Toggle = static_cast<FrogPilotParamValueControl*>(toggles["Offset7"]);
  FrogPilotParamValueControl *increasedStoppedDistanceToggle = static_cast<FrogPilotParamValueControl*>(toggles["IncreasedStoppedDistance"]);
  FrogPilotParamValueControl *increasedStoppedDistanceLowVisibilityToggle = static_cast<FrogPilotParamValueControl*>(toggles["IncreasedStoppedDistanceLowVisibility"]);
  FrogPilotParamValueControl *increasedStoppedDistanceRainToggle = static_cast<FrogPilotParamValueControl*>(toggles["IncreasedStoppedDistanceRain"]);
  FrogPilotParamValueControl *increasedStoppedDistanceRainStormToggle = static_cast<FrogPilotParamValueControl*>(toggles["IncreasedStoppedDistanceRainStorm"]);
  FrogPilotParamValueControl *increasedStoppedDistanceSnowToggle = static_cast<FrogPilotParamValueControl*>(toggles["IncreasedStoppedDistanceSnow"]);
  FrogPilotParamValueControl *setSpeedOffsetToggle = static_cast<FrogPilotParamValueControl*>(toggles["SetSpeedOffset"]);

  if (metric) {
    offset1Toggle->setTitle(tr("Speed Offset (0-29 km/h)"));
    offset2Toggle->setTitle(tr("Speed Offset (30-49 km/h)"));
    offset3Toggle->setTitle(tr("Speed Offset (50-59 km/h)"));
    offset4Toggle->setTitle(tr("Speed Offset (60-79 km/h)"));
    offset5Toggle->setTitle(tr("Speed Offset (80-99 km/h)"));
    offset6Toggle->setTitle(tr("Speed Offset (100-119 km/h)"));
    offset7Toggle->setTitle(tr("Speed Offset (120-140 km/h)"));

    offset1Toggle->setDescription(tr("<b>Adds to or subtracts from the posted speed limit in this range</b>, so openpilot cruises that much faster or slower than the sign. Use a positive value to flow with traffic above the limit, or a negative value to stay under it."));
    offset2Toggle->setDescription(tr("<b>Adds to or subtracts from the posted speed limit in this range</b>, so openpilot cruises that much faster or slower than the sign. Use a positive value to flow with traffic above the limit, or a negative value to stay under it."));
    offset3Toggle->setDescription(tr("<b>Adds to or subtracts from the posted speed limit in this range</b>, so openpilot cruises that much faster or slower than the sign. Use a positive value to flow with traffic above the limit, or a negative value to stay under it."));
    offset4Toggle->setDescription(tr("<b>Adds to or subtracts from the posted speed limit in this range</b>, so openpilot cruises that much faster or slower than the sign. Use a positive value to flow with traffic above the limit, or a negative value to stay under it."));
    offset5Toggle->setDescription(tr("<b>Adds to or subtracts from the posted speed limit in this range</b>, so openpilot cruises that much faster or slower than the sign. Use a positive value to flow with traffic above the limit, or a negative value to stay under it."));
    offset6Toggle->setDescription(tr("<b>Adds to or subtracts from the posted speed limit in this range</b>, so openpilot cruises that much faster or slower than the sign. Use a positive value to flow with traffic above the limit, or a negative value to stay under it."));
    offset7Toggle->setDescription(tr("<b>Adds to or subtracts from the posted speed limit in this range</b>, so openpilot cruises that much faster or slower than the sign. Use a positive value to flow with traffic above the limit, or a negative value to stay under it."));

    increasedStoppedDistanceToggle->updateControl(0, 3, metricDistanceLabels);
    increasedStoppedDistanceLowVisibilityToggle->updateControl(0, 3, metricDistanceLabels);
    increasedStoppedDistanceRainToggle->updateControl(0, 3, metricDistanceLabels);
    increasedStoppedDistanceRainStormToggle->updateControl(0, 3, metricDistanceLabels);
    increasedStoppedDistanceSnowToggle->updateControl(0, 3, metricDistanceLabels);

    ceSignal->updateControl(0, 150, metricSpeedLabels);
    ceSpeedToggle->updateControl(0, 150, metricSpeedLabels);
    customCruiseToggle->updateControl(1, 150, metricSpeedLabels);
    customCruiseLongToggle->updateControl(1, 150, metricSpeedLabels);
    offset1Toggle->updateControl(-150, 150, metricSpeedLabels);
    offset2Toggle->updateControl(-150, 150, metricSpeedLabels);
    offset3Toggle->updateControl(-150, 150, metricSpeedLabels);
    offset4Toggle->updateControl(-150, 150, metricSpeedLabels);
    offset5Toggle->updateControl(-150, 150, metricSpeedLabels);
    offset6Toggle->updateControl(-150, 150, metricSpeedLabels);
    offset7Toggle->updateControl(-150, 150, metricSpeedLabels);
    setSpeedOffsetToggle->updateControl(-150, 150, metricSpeedLabels);
  } else {
    offset1Toggle->setTitle(tr("Speed Offset (0-24 mph)"));
    offset2Toggle->setTitle(tr("Speed Offset (25-34 mph)"));
    offset3Toggle->setTitle(tr("Speed Offset (35-44 mph)"));
    offset4Toggle->setTitle(tr("Speed Offset (45-54 mph)"));
    offset5Toggle->setTitle(tr("Speed Offset (55-64 mph)"));
    offset6Toggle->setTitle(tr("Speed Offset (65-74 mph)"));
    offset7Toggle->setTitle(tr("Speed Offset (75-99 mph)"));

    offset1Toggle->setDescription(tr("<b>Adds to or subtracts from the posted speed limit in this range</b>, so openpilot cruises that much faster or slower than the sign. Use a positive value to flow with traffic above the limit, or a negative value to stay under it."));
    offset2Toggle->setDescription(tr("<b>Adds to or subtracts from the posted speed limit in this range</b>, so openpilot cruises that much faster or slower than the sign. Use a positive value to flow with traffic above the limit, or a negative value to stay under it."));
    offset3Toggle->setDescription(tr("<b>Adds to or subtracts from the posted speed limit in this range</b>, so openpilot cruises that much faster or slower than the sign. Use a positive value to flow with traffic above the limit, or a negative value to stay under it."));
    offset4Toggle->setDescription(tr("<b>Adds to or subtracts from the posted speed limit in this range</b>, so openpilot cruises that much faster or slower than the sign. Use a positive value to flow with traffic above the limit, or a negative value to stay under it."));
    offset5Toggle->setDescription(tr("<b>Adds to or subtracts from the posted speed limit in this range</b>, so openpilot cruises that much faster or slower than the sign. Use a positive value to flow with traffic above the limit, or a negative value to stay under it."));
    offset6Toggle->setDescription(tr("<b>Adds to or subtracts from the posted speed limit in this range</b>, so openpilot cruises that much faster or slower than the sign. Use a positive value to flow with traffic above the limit, or a negative value to stay under it."));
    offset7Toggle->setDescription(tr("<b>Adds to or subtracts from the posted speed limit in this range</b>, so openpilot cruises that much faster or slower than the sign. Use a positive value to flow with traffic above the limit, or a negative value to stay under it."));

    increasedStoppedDistanceToggle->updateControl(0, 10, imperialDistanceLabels);
    increasedStoppedDistanceLowVisibilityToggle->updateControl(0, 10, imperialDistanceLabels);
    increasedStoppedDistanceRainToggle->updateControl(0, 10, imperialDistanceLabels);
    increasedStoppedDistanceRainStormToggle->updateControl(0, 10, imperialDistanceLabels);
    increasedStoppedDistanceSnowToggle->updateControl(0, 10, imperialDistanceLabels);

    ceSignal->updateControl(0, 99, imperialSpeedLabels);
    ceSpeedToggle->updateControl(0, 99, imperialSpeedLabels);
    customCruiseToggle->updateControl(1, 99, imperialSpeedLabels);
    customCruiseLongToggle->updateControl(1, 99, imperialSpeedLabels);
    offset1Toggle->updateControl(-99, 99, imperialSpeedLabels);
    offset2Toggle->updateControl(-99, 99, imperialSpeedLabels);
    offset3Toggle->updateControl(-99, 99, imperialSpeedLabels);
    offset4Toggle->updateControl(-99, 99, imperialSpeedLabels);
    offset5Toggle->updateControl(-99, 99, imperialSpeedLabels);
    offset6Toggle->updateControl(-99, 99, imperialSpeedLabels);
    offset7Toggle->updateControl(-99, 99, imperialSpeedLabels);
    setSpeedOffsetToggle->updateControl(0, 99, imperialSpeedLabels);
  }
}

void FrogPilotLongitudinalPanel::updateToggles() {
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

    if (key == "CEStopLights") {
      setVisible &= !toggles["CEModelStopTime"]->isVisible();
    }

    else if (key == "CustomCruise" || key == "CustomCruiseLong" || key == "SetSpeedLimit" || key == "SetSpeedOffset") {
      setVisible &= !parent->hasPCMCruise;
    }

    else if (key == "ForceMPHDashboard") {
      setVisible &= parent->isToyota;
    }

    else if (key == "HumanLaneChanges") {
      setVisible &= parent->hasRadar;
    }

    else if (key == "MapGears") {
      setVisible &= parent->isGM || parent->isHKGCanFd || parent->isToyota;
      setVisible &= !parent->isTSK;
    }

    else if (key == "ReverseCruise") {
      setVisible &= parent->isToyota;
    }

    else if (key == "SLCMapboxFiller") {
      setVisible &= !params.get("MapboxSecretKey").empty();
    }

    else if (key == "StartAccel") {
      setVisible &= !(params.getBool("LongitudinalTune") && params.getBool("HumanAcceleration"));
    }

    else if (key == "StoppingDecelRate" || key == "VEgoStarting" || key == "VEgoStopping") {
      setVisible &= !parent->isGM || !params.getBool("ExperimentalGMTune");
      setVisible &= !parent->isToyota || !params.getBool("FrogsGoMoosTweak");
    }

    toggle->setVisible(setVisible);

    if (setVisible) {
      if (advancedLongitudinalTuneKeys.contains(key)) {
        toggles["AdvancedLongitudinalTune"]->setVisible(true);
      } else if (aggressivePersonalityKeys.contains(key)) {
        toggles["AggressivePersonalityProfile"]->setVisible(true);
      } else if (conditionalExperimentalKeys.contains(key)) {
        toggles["ConditionalExperimental"]->setVisible(true);
      } else if (curveSpeedKeys.contains(key)) {
        toggles["CurveSpeedController"]->setVisible(true);
      } else if (customDrivingPersonalityKeys.contains(key)) {
        toggles["CustomPersonalities"]->setVisible(true);
      } else if (longitudinalTuneKeys.contains(key)) {
        toggles["LongitudinalTune"]->setVisible(true);
      } else if (qolKeys.contains(key)) {
        toggles["QOLLongitudinal"]->setVisible(true);
      } else if (relaxedPersonalityKeys.contains(key)) {
        toggles["RelaxedPersonalityProfile"]->setVisible(true);
      } else if (speedLimitControllerKeys.contains(key)) {
        toggles["SpeedLimitController"]->setVisible(true);
      } else if (speedLimitControllerOffsetsKeys.contains(key)) {
        toggles["SLCOffsets"]->setVisible(true);
      } else if (speedLimitControllerQOLKeys.contains(key)) {
        toggles["SLCQOL"]->setVisible(true);
      } else if (speedLimitControllerVisualKeys.contains(key)) {
        toggles["SLCVisuals"]->setVisible(true);
      } else if (standardPersonalityKeys.contains(key)) {
        toggles["StandardPersonalityProfile"]->setVisible(true);
      }
    }
  }

  openDescriptions(forceOpenDescriptions, toggles);

  update();
}
