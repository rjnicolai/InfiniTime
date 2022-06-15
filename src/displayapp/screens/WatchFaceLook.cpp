#include "displayapp/screens/WatchFaceLook.h"

#include <date/date.h>
#include <lvgl/lvgl.h>
#include <cstdio>
#include "displayapp/screens/BatteryIcon.h"
#include "displayapp/screens/BleIcon.h"
#include "displayapp/screens/NotificationIcon.h"
#include "displayapp/screens/Symbols.h"
#include "components/battery/BatteryController.h"
#include "components/ble/BleController.h"
#include "components/ble/NotificationManager.h"
#include "components/heartrate/HeartRateController.h"
#include "components/motion/MotionController.h"
#include "components/settings/Settings.h"
using namespace Pinetime::Applications::Screens;

WatchFaceLook::WatchFaceLook(DisplayApp* app,
                             Controllers::DateTime& dateTimeController,
                             Controllers::Battery& batteryController,
                             Controllers::Ble& bleController,
                             Controllers::NotificationManager& notificatioManager,
                             Controllers::Settings& settingsController,
                             Controllers::HeartRateController& heartRateController,
                             Controllers::MotionController& motionController)
  : Screen(app),
    currentDateTime {{}},
    dateTimeController {dateTimeController},
    batteryController {batteryController},
    bleController {bleController},
    notificatioManager {notificatioManager},
    settingsController {settingsController},
    heartRateController {heartRateController},
    motionController {motionController} {
  settingsController.SetClockFace(5);

  batteryIcon = lv_label_create(lv_scr_act(), nullptr);
  lv_label_set_text_static(batteryIcon, Symbols::batteryFull);
  lv_obj_align(batteryIcon, lv_scr_act(), LV_ALIGN_IN_TOP_RIGHT, 0, 0);

  batteryPlug = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_text_color(batteryPlug, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0xFF0000));
  lv_label_set_text_static(batteryPlug, Symbols::plug);
  lv_obj_align(batteryPlug, batteryIcon, LV_ALIGN_OUT_LEFT_MID, -5, 0);

  bleIcon = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_text_color(bleIcon, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0x0082FC));
  lv_label_set_text_static(bleIcon, Symbols::bluetooth);
  lv_obj_align(bleIcon, batteryPlug, LV_ALIGN_OUT_LEFT_MID, -5, 0);

  notificationIcon = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_text_color(notificationIcon, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0x00FF00));
  lv_label_set_text_static(notificationIcon, NotificationIcon::GetIcon(false));
  lv_obj_align(notificationIcon, nullptr, LV_ALIGN_IN_TOP_LEFT, 0, 0);

  label_title = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_text_color(label_title, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0x795E93));
  lv_label_set_text_static(label_title, "F&V LOOK");
  lv_obj_align(label_title, lv_scr_act(), LV_ALIGN_CENTER, 0, 0);

  label_subtitle = lv_label_create(lv_scr_act(), nullptr);
  lv_label_set_text_static(label_subtitle, "Smartwatch #7");
  lv_obj_align(label_subtitle, lv_scr_act(), LV_ALIGN_CENTER, 0, 24);
  lv_obj_set_style_local_text_color(label_subtitle, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0x009870));

  backgroundLabel = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_set_click(backgroundLabel, true);
  lv_label_set_long_mode(backgroundLabel, LV_LABEL_LONG_CROP);
  lv_obj_set_size(backgroundLabel, 240, 240);
  lv_obj_set_pos(backgroundLabel, 0, 0);
  lv_label_set_text_static(backgroundLabel, "");

  taskRefresh = lv_task_create(RefreshTaskCallback, LV_DISP_DEF_REFR_PERIOD, LV_TASK_PRIO_MID, this);
  Refresh();
}

WatchFaceLook::~WatchFaceLook() {
  lv_task_del(taskRefresh);
  lv_obj_clean(lv_scr_act());
}

void WatchFaceLook::Refresh() {
  powerPresent = batteryController.IsPowerPresent();
  if (powerPresent.IsUpdated()) {
    lv_label_set_text_static(batteryPlug, BatteryIcon::GetPlugIcon(powerPresent.Get()));
  }

  batteryPercentRemaining = batteryController.PercentRemaining();
  if (batteryPercentRemaining.IsUpdated()) {
    auto batteryPercent = batteryPercentRemaining.Get();
    if (batteryPercent == 100) {
      lv_obj_set_style_local_text_color(batteryIcon, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_GREEN);
    } else {
      lv_obj_set_style_local_text_color(batteryIcon, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_WHITE);
    }
    lv_label_set_text_static(batteryIcon, BatteryIcon::GetBatteryIcon(batteryPercent));
  }

  bleState = bleController.IsConnected();
  bleRadioEnabled = bleController.IsRadioEnabled();
  if (bleState.IsUpdated() || bleRadioEnabled.IsUpdated()) {
    lv_label_set_text(bleIcon, BleIcon::GetIcon(bleState.Get()));
  }
  lv_obj_realign(batteryIcon);
  lv_obj_realign(batteryPlug);
  lv_obj_realign(bleIcon);

  notificationState = notificatioManager.AreNewNotificationsAvailable();
  if (notificationState.IsUpdated()) {
    lv_label_set_text_static(notificationIcon, NotificationIcon::GetIcon(notificationState.Get()));
  }

  // currentDateTime = dateTimeController.CurrentDateTime();

  // if (currentDateTime.IsUpdated()) {
  //   auto newDateTime = currentDateTime.Get();

  //   auto dp = date::floor<date::days>(newDateTime);
  //   auto time = date::make_time(newDateTime - dp);
  //   auto yearMonthDay = date::year_month_day(dp);

  //   auto year = static_cast<int>(yearMonthDay.year());
  //   auto month = static_cast<Pinetime::Controllers::DateTime::Months>(static_cast<unsigned>(yearMonthDay.month()));
  //   auto day = static_cast<unsigned>(yearMonthDay.day());
  //   auto dayOfWeek = static_cast<Pinetime::Controllers::DateTime::Days>(date::weekday(yearMonthDay).iso_encoding());

  //   uint8_t hour = time.hours().count();
  //   uint8_t minute = time.minutes().count();

  //   if (displayedHour != hour || displayedMinute != minute) {
  //     displayedHour = hour;
  //     displayedMinute = minute;

  //     if (settingsController.GetClockType() == Controllers::Settings::ClockType::H12) {
  //       char ampmChar[3] = "AM";
  //       if (hour == 0) {
  //         hour = 12;
  //       } else if (hour == 12) {
  //         ampmChar[0] = 'P';
  //       } else if (hour > 12) {
  //         hour = hour - 12;
  //         ampmChar[0] = 'P';
  //       }
  //       lv_label_set_text(label_time_ampm, ampmChar);
  //       lv_label_set_text_fmt(label_time, "%2d:%02d", hour, minute);
  //       lv_obj_align(label_time, lv_scr_act(), LV_ALIGN_IN_RIGHT_MID, 0, 0);
  //     } else {
  //       lv_label_set_text_fmt(label_time, "%02d:%02d", hour, minute);
  //       lv_obj_align(label_time, lv_scr_act(), LV_ALIGN_CENTER, 0, 0);
  //     }
  //   }

  //   if ((year != currentYear) || (month != currentMonth) || (dayOfWeek != currentDayOfWeek) || (day != currentDay)) {
  //     if (settingsController.GetClockType() == Controllers::Settings::ClockType::H24) {
  //       lv_label_set_text_fmt(
  //         label_date, "%s %d %s %d", dateTimeController.DayOfWeekShortToString(), day, dateTimeController.MonthShortToString(), year);
  //     } else {
  //       lv_label_set_text_fmt(
  //         label_date, "%s %s %d %d", dateTimeController.DayOfWeekShortToString(), dateTimeController.MonthShortToString(), day, year);
  //     }
  //     lv_obj_realign(label_date);

  //     currentYear = year;
  //     currentMonth = month;
  //     currentDayOfWeek = dayOfWeek;
  //     currentDay = day;
  //   }
  // }

  // heartbeat = heartRateController.HeartRate();
  // heartbeatRunning = heartRateController.State() != Controllers::HeartRateController::States::Stopped;
  // if (heartbeat.IsUpdated() || heartbeatRunning.IsUpdated()) {
  //   if (heartbeatRunning.Get()) {
  //     lv_obj_set_style_local_text_color(heartbeatIcon, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0xCE1B1B));
  //     lv_label_set_text_fmt(heartbeatValue, "%d", heartbeat.Get());
  //   } else {
  //     lv_obj_set_style_local_text_color(heartbeatIcon, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0x1B1B1B));
  //     lv_label_set_text_static(heartbeatValue, "");
  //   }

  //   lv_obj_realign(heartbeatIcon);
  //   lv_obj_realign(heartbeatValue);
  // }

  // stepCount = motionController.NbSteps();
  // motionSensorOk = motionController.IsSensorOk();
  // if (stepCount.IsUpdated() || motionSensorOk.IsUpdated()) {
  //   lv_label_set_text_fmt(stepValue, "%lu", stepCount.Get());
  //   lv_obj_realign(stepValue);
  //   lv_obj_realign(stepIcon);
  // }
}
